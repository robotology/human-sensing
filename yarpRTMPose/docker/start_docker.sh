IMG=fbrand-new/yarprtmpose:devel_cudnn

docker run -it --rm --privileged --gpus=all --network host --pid host\
  -v /etc/localtime:/etc/localtime -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $HOME/.Xauthority:/root/.Xauthority -e DISPLAY=$DISPLAY\
  -e QT_X11_NO_MITSHM=1 -v /etc/hosts:/etc/hosts -v ~/misc:/root/misc \
  $IMG bash
