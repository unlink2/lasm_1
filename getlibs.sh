#!/bin/sh

function getHeaderOnlyLib() {
    # get libraries
    mkdir -p ./libs
    cd ./libs
    if [ -d $1 ]; then
        cd $1
        git pull
    else
        git clone $2
    fi

    # back
    cd ../../
}

getHeaderOnlyLib "argcc" "https://github.com/unlink2/argcc"
