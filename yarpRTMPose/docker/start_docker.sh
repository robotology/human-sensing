if [[ -z $1 ]]
then
  DATA_DIR_PATH=~/Videos
else
  DATA_DIR_PATH=$1
fi

if [[ -z $2 ]]
then
  IMG=fbrand-new/yarprtmpose:devel_cudnn
else
  IMG=$2
fi

DATA_DIR=$(basename ${DATA_DIR_PATH})

docker run -it --rm --privileged --gpus=all --network host --pid host\
  -v /etc/localtime:/etc/localtime -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v ${XAUTHORITY}:/root/.Xauthority -e DISPLAY=$DISPLAY \
  -e QT_X11_NO_MITSHM=1 -v /etc/hosts:/etc/hosts -v $DATA_DIR_PATH:/${DATA_DIR} \
  $IMG bash
