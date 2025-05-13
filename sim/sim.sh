#!/bin/bash

MODE=$1
ARG1=$2
ARG2=$3


ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if [[ "$MODE" == "security" ]]; then
    echo "=== Running Security Test ==="
    cd "$ROOT_DIR/Security/src" || exit 1
    make || { echo "Security make failed"; exit 1; }

    cd "$ROOT_DIR/Security/bin" || exit 1
    ./main "$ARG1" "$ARG2" || { echo "Security test failed"; exit 1; }

elif [[ "$MODE" == "reliability" ]]; then
    echo "=== Running Reliability Test ==="
    cd "$ROOT_DIR/Reliability" || exit 1
    make clean
    make || { echo "Reliability make failed"; exit 1; }
    python3 run.py || { echo "Reliability test failed"; exit 1; }

else
    echo "Usage:"
    echo "  ./sim.sh security [arg1] [arg2]"
    echo "  ./sim.sh reliability"
    exit 1
fi