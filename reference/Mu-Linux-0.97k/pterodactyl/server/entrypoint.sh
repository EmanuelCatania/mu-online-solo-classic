#!/bin/bash
set -euo pipefail

cd /home/container
exec /bin/bash -c "${STARTUP:-./start.sh}"
