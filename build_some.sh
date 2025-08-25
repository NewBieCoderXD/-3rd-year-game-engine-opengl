#!/usr/bin/env bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

diff=$(realpath -m --relative-to="$SCRIPT_DIR/src" -- "$1")
IFS='/' read -ra path_parts <<< "$diff"
chapter=${path_parts[0]}
demo=${path_parts[1]}

cd $SCRIPT_DIR
mkdir -p build
cd build
INPUT_CHAPTER="$chapter" INPUT_DEMO="$demo" cmake ..
cmake --build .

cd ../bin/$chapter

./${chapter}__${demo}