ARG PYTORCH="2.0.1"
ARG CUDA="11.7"
ARG CUDNN="8"

ARG CMAKE_GENERATOR="Unix Makefiles"
ARG BUILD_TYPE="Release"
ARG CMAKE_EXTRA_OPTIONS=-j8
ARG INSTALL_DIR="/usr/local"
ARG SUPERBUILD_RELEASE="releases/2023.02"

FROM pytorch/pytorch:${PYTORCH}-cuda${CUDA}-cudnn${CUDNN}-devel
# FROM pytorch/pytorch:latest

ENV TORCH_CUDA_ARCH_LIST="6.0 6.1 7.0+PTX"
ENV TORCH_NVCC_FLAGS="-Xfatbin -compress-all"
ENV CMAKE_PREFIX_PATH="$(dirname $(which conda))/../"

# Non interactive installation mode
ENV DEBIAN_FRONTEND=noninteractive

# To fix GPG key error when running apt-get update
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/7fa2af80.pub

RUN apt-get update && apt-get install -y wget git ninja-build libglib2.0-0 libsm6 libxrender-dev libxext6 libgl1-mesa-glx vim
    # && apt-get clean \
    # && rm -rf /var/lib/apt/lists/*

# Install xtcocotools
RUN pip install cython
RUN pip install xtcocotools

# Install MMEngine and MMCV
RUN pip install openmim
RUN mim install mmengine "mmcv>=2.0.0"

# Install MMPose
RUN conda clean --all
RUN git clone https://github.com/open-mmlab/mmpose.git /mmpose
WORKDIR /mmpose
RUN git checkout main
ENV FORCE_CUDA="1"
RUN pip install -r requirements/build.txt
RUN pip install --no-cache-dir -e .
RUN pip install mmdet
RUN pip install opencv-python

# Install yarp
RUN apt update && apt install -y cmake g++ wget unzip \
    build-essential git cmake cmake-curses-gui swig \
    libeigen3-dev \
    libace-dev \
    libedit-dev \
    libsqlite3-dev \
    libtinyxml-dev \
    qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev \
    qml-module-qtquick2 qml-module-qtquick-window2 \
    qml-module-qtmultimedia qml-module-qtquick-dialogs \
    qml-module-qtquick-controls qml-module-qt-labs-folderlistmodel \
    qml-module-qt-labs-settings \
    libqcustomplot-dev \
    libgraphviz-dev \
    libjpeg-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav

RUN cd / && git clone https://github.com/robotology/ycm.git && mkdir robotology && mv ycm/ robotology/ \
    && cd robotology/ycm && mkdir build \
    && cd build && cmake .. && make && make install

RUN cd /robotology && git clone https://github.com/robotology/yarp.git \
	&& cd yarp && mkdir build && cd build \ 
    && cmake -D ENABLE_yarpcar_mjpeg=ON -D YARP_COMPILE_BINDINGS=ON -D CREATE_PYTHON=ON .. \
	&& make -j8 && make install

RUN cd /robotology && git clone https://github.com/robotology/icub-main.git \
	&& cd icub-main && mkdir build && cd build && cmake .. && make && make install 
RUN cd /robotology && git clone https://github.com/robotology/icub-contrib-common.git \
	&& cd icub-contrib-common && mkdir build && cd build && cmake .. && make && make install

# Install the application
COPY . /yarpPose
RUN /yarpPose/download_models.sh

ENV PYTHONPATH="/robotology/yarp/build/lib/python3:/root/yarpPose"

COPY docker/entrypoint.sh /
ENTRYPOINT ["/bin/bash", "/entrypoint.sh"] 
