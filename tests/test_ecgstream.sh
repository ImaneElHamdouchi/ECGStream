#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
mkdir -p out

./ecgstream analyze samples/normal.csv --json out/normal.json > out/normal.txt
grep -q '"classification": "normal"' out/normal.json
grep -q '"bradycardia": false' out/normal.json
grep -q '"tachycardia": false' out/normal.json

./ecgstream analyze samples/tachycardia.csv --json out/tachycardia.json > out/tachycardia.txt
grep -q '"classification": "tachycardia"' out/tachycardia.json
grep -q '"tachycardia": true' out/tachycardia.json

./ecgstream analyze samples/bradycardia.csv --json out/bradycardia.json > out/bradycardia.txt
grep -q '"classification": "bradycardia"' out/bradycardia.json
grep -q '"bradycardia": true' out/bradycardia.json

./ecgstream analyze samples/irregular.csv --json out/irregular.json > out/irregular.txt
grep -q '"classification": "irregular"' out/irregular.json
grep -q '"irregular": true' out/irregular.json

echo "PASS ECGStream tests"
