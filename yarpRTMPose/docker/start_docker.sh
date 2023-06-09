#!/bin/bash

DATA_DIR_PATH=~/Videos
IMG=fbrand-new/yarprtmpose:devel_cudnn
CMD=bash
CONTAINER_NAME=yarprtmpose

while getopts ":i:d:c:n:" opt; do
  case $opt in
    i)
        IMG="$OPTARG"
    ;;
    d)
        DATA_DIR_PATH="$OPTARG"
    ;;
    c)
        CMD="$OPTARG"
    ;;
    n)
        CONTAINER_NAME="${OPTARG}"
    ;;
    \?) echo "Invalid option -$OPTARG" >&3
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

DATA_DIR=$(basename ${DATA_DIR_PATH})

docker run -it --rm --privileged --gpus=all --network host --pid host\
  -v /etc/localtime:/etc/localtime -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v ${XAUTHORITY}:/root/.Xauthority -e DISPLAY=$DISPLAY \
  -e QT_X11_NO_MITSHM=1 -v /etc/hosts:/etc/hosts -v $DATA_DIR_PATH:/${DATA_DIR} \
  --name $CONTAINER_NAME $IMG $CMD

