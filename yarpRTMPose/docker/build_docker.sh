#!/bin/bash

usage() { echo "Usage: $0 [-i image_name]
                " 1>&2; exit 1; }

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

IMG=fbrand-new/yarprtmpose:devel_cudnn

while getopts ":i:d" o; do
    case "${o}" in
        i)
            IMG=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done

cd $SCRIPT_DIR

docker build -t $IMG ..
