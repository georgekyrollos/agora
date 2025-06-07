#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <wallet.dat path>"
    exit 1
fi

WALLET_PATH="$1"

while true; do
    ./miner "$WALLET_PATH"
    sleep 3 
done
