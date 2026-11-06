/* Host unit tests for pedelec.c (motor relay decision logic).
   Build and run:  make -C test   (or: cc -Itest/host -I. test/test_pedelec.c pedelec.c) */
#include <stdio.h>
#include "xtypes.h"
#include "pedelec.h"

static int g_failed = 0;
static int g_run = 0;

#define CHECK(cond, msg) do { g_run++; if (!(cond)) { g_failed++; \
  printf("FAIL line %d: %s\n", __LINE__, msg); } } while (0)

/* default inputs: pedelec mode, pedalling, 15 km/h, limits 3.0 .. 25.0 km/h */
static S_PEDELEC_INPUT in_at(UINT32 now, UINT32 speed)
{
  S_PEDELEC_INPUT in;
  in.u32Now = now;
  in.bSafetyCut = FALSE;
  in.bPedelecMode = TRUE;
  in.bPedalling = TRUE;
  in.u32Speed = speed;
  in.u16SpeedMin = 30U;
  in.u16SpeedMax = 250U;
  return in;
}

/* run the decision every 10 ms from t0 to t1 with the same inputs */
static E_BOOL run(S_PEDELEC_STATE *s, S_PEDELEC_INPUT in, UINT32 t0, UINT32 t1)
{
  E_BOOL r = FALSE;
  UINT32 steps = (UINT32)(t1 - t0) / 10U;
  for (UINT32 i = 0U; i < steps; i++) { in.u32Now = (UINT32)(t0 + i * 10U); r = pedelec_Decide(s, &in); }
  return r;
}

static void test_start_and_dwell(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  S_PEDELEC_INPUT in = in_at(100U, 150U);
  CHECK(pedelec_Decide(&s, &in) == FALSE, "no switch within 500 ms of init");
  in.u32Now = 500U;
  CHECK(pedelec_Decide(&s, &in) == TRUE, "motor on after dwell when pedalling at 15 km/h");
}

static void test_min_speed_hysteresis(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  CHECK(run(&s, in_at(0, 29U), 0U, 1000U) == FALSE, "no start below 3.0 km/h");
  CHECK(run(&s, in_at(0, 30U), 1000U, 2000U) == TRUE, "start at 3.0 km/h");
  CHECK(run(&s, in_at(0, 26U), 2000U, 3000U) == TRUE, "stays on at 2.6 km/h (hysteresis 0.5)");
  CHECK(run(&s, in_at(0, 25U), 3000U, 4000U) == TRUE, "stays on at exactly 2.5 km/h");
  CHECK(run(&s, in_at(0, 24U), 4000U, 5000U) == FALSE, "off below 2.5 km/h");
  CHECK(run(&s, in_at(0, 29U), 5000U, 6000U) == FALSE, "no restart at 2.9 km/h");
}

static void test_max_speed_hysteresis(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  CHECK(run(&s, in_at(0, 245U), 0U, 1000U) == FALSE, "no start at 24.5 km/h (needs <= 24.0)");
  CHECK(run(&s, in_at(0, 240U), 1000U, 2000U) == TRUE, "start at 24.0 km/h");
  CHECK(run(&s, in_at(0, 250U), 2000U, 3000U) == TRUE, "stays on at 25.0 km/h");
  CHECK(run(&s, in_at(0, 251U), 3000U, 4000U) == FALSE, "off above 25.0 km/h");
  CHECK(run(&s, in_at(0, 245U), 4000U, 5000U) == FALSE, "stays off at 24.5 km/h");
  CHECK(run(&s, in_at(0, 240U), 5000U, 6000U) == TRUE, "back on at 24.0 km/h");
}

static void test_no_chatter_at_limit(void)
{
  /* speed jittering around 25.0 km/h every 10 ms: count relay changes */
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  run(&s, in_at(0, 200U), 0U, 1000U);
  int changes = 0;
  E_BOOL last = s.bMotorOn;
  for (UINT32 t = 1000U; t < 11000U; t += 10U)
  {
    S_PEDELEC_INPUT in = in_at(t, ((t / 10U) % 2U) ? 249U : 252U);
    E_BOOL r = pedelec_Decide(&s, &in);
    if (r != last) { changes++; last = r; }
  }
  /* jitter 24.9..25.2 never gets below 24.0, so: one switch-off, then off */
  CHECK(changes == 1, "jitter around the max limit switches off once, no chatter");
}

static void test_dwell_limits_switching(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  run(&s, in_at(0, 150U), 0U, 1000U);          /* on at t = 500 */
  int changes = 0;
  E_BOOL last = s.bMotorOn;
  for (UINT32 t = 1000U; t < 3000U; t += 10U)  /* pedalling flips every 50 ms */
  {
    S_PEDELEC_INPUT in = in_at(t, 150U);
    in.bPedalling = ((t / 50U) % 2U) ? TRUE : FALSE;
    E_BOOL r = pedelec_Decide(&s, &in);
    if (r != last) { changes++; last = r; }
  }
  CHECK(changes <= 4, "at most one relay change per 500 ms");
}

static void test_safety_cut_is_immediate(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  run(&s, in_at(0, 150U), 0U, 1000U);          /* switched on at t = 500 */
  S_PEDELEC_INPUT in = in_at(510U, 150U);
  in.u32Now = 1000U;
  in.bSafetyCut = TRUE;
  CHECK(pedelec_Decide(&s, &in) == FALSE, "safety cut acts immediately");
  /* and right after switching on (inside the dwell time) too */
  pedelec_Init(&s, 0U);
  run(&s, in_at(0, 150U), 0U, 510U);           /* switched on at t = 500 */
  in = in_at(505U, 150U);
  in.u32Now = 510U;
  in.bSafetyCut = TRUE;
  CHECK(pedelec_Decide(&s, &in) == FALSE, "safety cut ignores the dwell time");
}

static void test_stop_pedalling(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  run(&s, in_at(0, 150U), 0U, 1000U);
  S_PEDELEC_INPUT in = in_at(0, 150U);
  in.bPedalling = FALSE;
  CHECK(run(&s, in, 1000U, 1600U) == FALSE, "motor off when pedalling stops");
}

static void test_not_pedelec_mode(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  S_PEDELEC_INPUT in = in_at(0, 0U);
  in.bPedelecMode = FALSE;
  in.bPedalling = FALSE;
  CHECK(run(&s, in, 0U, 1000U) == TRUE, "without pedelec mode the relay is enabled");
}

static void test_small_min_limit(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  S_PEDELEC_INPUT in = in_at(0, 3U);
  in.u16SpeedMin = 3U;                          /* 0.3 km/h, below hysteresis */
  CHECK(run(&s, in, 0U, 1000U) == TRUE, "start at tiny min limit");
  in.u32Speed = 0U;
  CHECK(run(&s, in, 1000U, 2000U) == TRUE, "no underflow: stays on at 0 with min 0.3");
}

static void test_no_max_limit(void)
{
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0U);
  S_PEDELEC_INPUT in = in_at(0, 400U);
  in.u16SpeedMax = PEDELEC_NO_MAX;
  CHECK(run(&s, in, 0U, 1000U) == TRUE, "max 0 = no limit: starts at 40 km/h");
  in.u32Speed = 600U;
  CHECK(run(&s, in, 1000U, 2000U) == TRUE, "max 0 = no limit: stays on at 60 km/h");
  in.u32Speed = 20U;
  CHECK(run(&s, in, 2000U, 3000U) == FALSE, "max 0: min limit still applies");
  in.u32Speed = 400U;
  in.bPedalling = FALSE;
  CHECK(run(&s, in, 3000U, 4000U) == FALSE, "max 0: still needs pedalling");
}

static void test_speed_bar(void)
{
  CHECK(pedelec_SpeedBarLevel(0U)   == 0U, "speed bar: standing still = no LED");
  CHECK(pedelec_SpeedBarLevel(49U)  == 0U, "speed bar: 4.9 km/h = no LED");
  CHECK(pedelec_SpeedBarLevel(50U)  == 1U, "speed bar: 5.0 km/h = 1 LED");
  CHECK(pedelec_SpeedBarLevel(149U) == 2U, "speed bar: 14.9 km/h = 2 LEDs");
  CHECK(pedelec_SpeedBarLevel(200U) == 4U, "speed bar: 20.0 km/h = 4 LEDs");
  CHECK(pedelec_SpeedBarLevel(250U) == 5U, "speed bar: 25.0 km/h = 5 LEDs");
  CHECK(pedelec_SpeedBarLevel(300U) == 5U, "speed bar: 30.0 km/h = 5 LEDs");
  CHECK(pedelec_SpeedBarLevel(301U) == SPEEDBAR_LEVEL_FLASH, "speed bar: above 30 km/h = flashing");
  CHECK(pedelec_SpeedBarLevel(0xFFFFFFFFU) == SPEEDBAR_LEVEL_FLASH, "speed bar: huge value = flashing");
}

static void test_time_wraparound(void)
{
  /* system time wraps after ~49.7 days */
  S_PEDELEC_STATE s;
  pedelec_Init(&s, 0xFFFFFE00U);
  S_PEDELEC_INPUT in = in_at(0, 150U);
  CHECK(run(&s, in, 0xFFFFFE00U, 0x00000400U) == TRUE, "dwell works across time wraparound");
}

int main(void)
{
  test_start_and_dwell();
  test_min_speed_hysteresis();
  test_max_speed_hysteresis();
  test_no_chatter_at_limit();
  test_dwell_limits_switching();
  test_safety_cut_is_immediate();
  test_stop_pedalling();
  test_not_pedelec_mode();
  test_small_min_limit();
  test_no_max_limit();
  test_speed_bar();
  test_time_wraparound();
  printf("%d checks, %d failed\n", g_run, g_failed);
  return g_failed ? 1 : 0;
}
