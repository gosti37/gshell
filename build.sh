#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
EDITLINE="$ROOT/external/editline"

cd "$EDITLINE"
./autogen.sh
./configure

cd "$ROOT"
xmake m -f release
xmake
