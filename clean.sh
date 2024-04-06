#!/bin/bash

# get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# clean
cd $DIR/src/parser && make clean || exit && cd ../..
make distclean
