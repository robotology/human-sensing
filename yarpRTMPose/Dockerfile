FROM nvidia/cuda:11.7.0-cudnn8-devel-ubuntu22.04

# Non interactive installation mode
ENV DEBIAN_FRONTEND=noninteractive

# To fix GPG key error when running apt-get update
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub
RUN apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/7fa2af80.pub

RUN apt-get update && apt-get install -y wget git ninja-build libglib2.0-0 libsm6 libxrender-dev libxext6 libgl1-mesa-glx vim
# compile python from source in order to get python3.8 - avoid unsupported library problems.
# I could not make mmdeploy work with python3.10 for now

RUN : \
    && apt-get update \ 
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        software-properties-common \
    && add-apt-repository -y ppa:deadsnakes \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        python3.8-dev \
        python3.8-venv \
        python3.8-distutils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && :

RUN python3.8 -m venv /venv
ENV PATH=/venv/bin:$PATH

# Install anaconda 
# RUN mkdir -p ~/miniconda3 \ 
#     && wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O ~/miniconda3/miniconda.sh \
#     && bash ~/miniconda3/miniconda.sh -b -u -p ~/miniconda3 \
#     && rm -rf ~/miniconda3/miniconda.sh 

# ENV PATH=/root/miniconda3/bin:${PATH}

# SHELL ["/bin/bash", "--login", "-c"]
# RUN echo ". $CONDA_DIR/etc/profild.d/conda.sh" >> ~/.profile
# RUN conda init bash
# RUN conda create --name openmmlab python=3.8 -y
# RUN echo "conda activate openmmlab" >> ~/.bashrc

# Install torch
# RUN conda install pytorch torchvision torchaudio pytorch-cuda=11.7 -c pytorch -c nvidia
RUN pip3 install torch torchvision torchaudio

# Install mmpose dependencies
RUN pip install -U openmim \
    && mim install mmengine \
    && mim install "mmcv>=2.0.0" \
    && mim install "mmdet>=3.0.0"

# Install mmpose
RUN git clone https://github.com/open-mmlab/mmpose.git \
    && cd mmpose \
    && pip install -r requirements.txt \
    && pip install -v -e .

# Install mmdeploy and onnx runtime
RUN pip install mmdeploy==1.0.0 \
    && pip install mmdeploy-runtime-gpu==1.0.0 \
    && pip install onnxruntime-gpu==1.8.1 \
    && wget https://github.com/microsoft/onnxruntime/releases/download/v1.8.1/onnxruntime-linux-x64-gpu-1.8.1.tgz \
    && tar -zxvf onnxruntime-linux-x64-gpu-1.8.1.tgz 

ENV ONNXRUNTIME_DIR=/onnxruntime-linux-x64-gpu-1.8.1
ENV LD_LIBRARY_PATH=${ONNXRUNTIME_DIR}/lib:$LD_LIBRARY_PATH

# Install yarp dependencies
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

# Install ffmpeg libraries for video grabber utils
RUN apt update && apt install -y ffmpeg \ 
        libavcodec-dev libavdevice-dev \ 
        libavformat-dev libavutil-dev

RUN cd / && git clone https://github.com/robotology/ycm.git && mkdir robotology && mv ycm/ robotology/ \
    && cd robotology/ycm && mkdir build \
    && cd build && cmake .. && make && make install

# Install yarp
RUN cd /robotology && git clone https://github.com/robotology/yarp.git \
	&& cd yarp && mkdir build && cd build \ 
    && cmake -D ENABLE_yarpcar_mjpeg=ON -D ENABLE_yarpmod_ffmpeg_grabber=ON \
        -D YARP_COMPILE_BINDINGS=ON -D CREATE_PYTHON=ON .. \
	&& make -j8 && make install

# Install other yarp libraries
RUN cd /robotology && git clone https://github.com/robotology/icub-main.git \
	&& cd icub-main && mkdir build && cd build && cmake .. && make && make install 
RUN cd /robotology && git clone https://github.com/robotology/icub-contrib-common.git \
	&& cd icub-contrib-common && mkdir build && cd build && cmake .. && make && make install

ENV PYTHONPATH="/robotology/yarp/build/lib/python3:/yarpRTMPose/src/python"
# We need tools and resources from mmdeploy 
RUN git clone -b main https://github.com/open-mmlab/mmdeploy.git

# Install the application
COPY . /yarpRTMPose
RUN /yarpRTMPose/app/scripts/deploy.sh