#!/bin/bash

# get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# compile
cd $DIR/src/parser && make && cd -
qmake $DIR
make
