#!/bin/bash

set -e

mkdir -p /app/build

find /app/src /app/include /app/CMakeLists.txt | entr -r bash -c '
    echo "Detected changes, rebuilding..."
    cd /app/build
    cmake ..
    make
    echo "Restarting server..."
    ./cap_returns --doc_root /app/www --port 8080
'