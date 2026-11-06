#!/bin/bash
# Double-click to start the C5 ULA Configurator (macOS).
# Serves this folder on http://localhost:8765 (Web Serial needs localhost or
# https) and opens it in Chrome, falling back to Edge or the default browser.
cd "$(dirname "$0")" || exit 1
PORT=8765
URL="http://localhost:$PORT/"

python3 -m http.server "$PORT" --bind 127.0.0.1 >/dev/null 2>&1 &
SERVER=$!
trap 'kill $SERVER 2>/dev/null' EXIT
sleep 1

if open -Ra "Google Chrome" 2>/dev/null; then
  open -a "Google Chrome" "$URL"
elif open -Ra "Microsoft Edge" 2>/dev/null; then
  open -a "Microsoft Edge" "$URL"
else
  echo "Chrome or Edge is needed for serial access (Safari/Firefox don't support Web Serial)."
  open "$URL"
fi

echo "C5 ULA Configurator running at $URL"
echo "Close this window to stop it."
wait $SERVER
