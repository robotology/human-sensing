# yarpOpenPose dependencies installation
This readme contains information on how to build and install all required dependencies for the yarpOpenPose module.

Table of Contents
=================
* [Requirements](#requirements)
* [Required Dependencies](#generic_dep)
* [OpenPose Installation](#openposeinstallation)
* [Compilation](#compilation)

#### Requirements

* **Ubuntu** (tested on 14 and 16) or **Windows** (tested on 10). We do not support any other OS but the community has been able to install it on: CentOS, Windows 7, and Windows 8.
* **NVIDIA graphics card** with at least 1.6 GB available (the nvidia-smi command checks the available GPU memory in Ubuntu).
* **CUDA** and **cuDNN** installed. Note: We found OpenPose working with cuDNN 5.1 ~10% faster than with cuDNN 6.
* At least **2 GB** of free **RAM** memory.
* Highly recommended: A **CPU** with at least **8 cores**.

Note: These requirements assume the default configuration (i.e. --net_resolution "656x368" and num_scales 1). You might need more (with a greater net resolution and/or number of scales) or less resources (with smaller net resolution and/or using the MPI and MPI_4 models).

#### Required Dependencies

- [YARP](https://github.com/robotology/yarp)
- [iCub](https://github.com/robotology/icub-main)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html)
- [openPose](https://github.com/CMU-Perceptual-Computing-Lab/openpose)
- [CUDA](https://developer.nvidia.com/cuda-downloads)
- [cuDNN](https://developer.nvidia.com/cudnn)
- [Caffe](http://caffe.berkeleyvision.org/installation.html)

#### OpenPose Installation

    $ git clone https://github.com/CMU-Perceptual-Computing-Lab/openpose.git

Follow the installation procedure at the following link [manual compilation](https://github.com/CMU-Perceptual-Computing-Lab/openpose/blob/master/doc/installation.md#installation---manual-compilation).


#### Compilation

While in the root folder of `yarpOpenPose` do:

    mkdir build
    cd build
    ccmake ..
