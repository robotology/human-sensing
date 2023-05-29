#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $SCRIPT_DIR

if [[ -z "$1" ]]
then
    IMG=fbrand-new/yarprtmpose:devel_cudnn
else
    IMG=$1
fi

docker build -t $IMG ..
