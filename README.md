# human-sensing-SAM
This repository contains software related to human sensing

The code has been tested on Ubuntu 14.04 LTS & Ubuntu 16.04 LTS

The code needs the follwoing libraries:

    1- BLAS (for dlib c++ library)
    2- OpenCV 3.0
    2- tbb
    3- boost
    4- dlib (for face detection)
    5- icub-main (implemented on icub)

## Get BLAS:

    sudo apt-get install libopenblas-dev liblapack-dev 

## Get TBB:

    sudo apt-get install libtbb-dev

## Get Boost:

    sudo apt-get install libboost-all-dev

### OpenCV-3.2.0
**`OpenCV-3.0.0`** or higher (**`OpenCV-3.2.0`** is recommended) is a required dependency:

1. Download `OpenCV`: `git clone https://github.com/opencv/opencv.git`.
2. Checkout the correct branch: `git checkout 3.2.0`.
3. Download the external modules: `git clone https://github.com/opencv/opencv_contrib.git`.
4. Checkout the correct branch: `git checkout 3.2.0`.
5. Configure `OpenCV` by filling in the cmake var **`OPENCV_EXTRA_MODULES_PATH`** with the path pointing to `opencv_contrib/modules` and then toggling on the var **`BUILD_opencv_tracking`**.
6. Compile `OpenCV`.

### YARP, icub-main and icub-contrib-common
First, follow the [installation instructions](http://wiki.icub.org/wiki/Linux:Installation_from_sources) for `yarp`, `icub-main` and `icub-contrib-common`.
    
## Get dlib:

Download the library (v18.xx) from the following link and install it:

    http://dlib.net/ 

finally, build the main code and test it by running the `SimpleCLM` executable. 

## Clone and build this repository

Add `CLM_MODEL_DIR=$SRC_FOLDER/human-sensing-SAM/app/CLM_Yarp/conf` to your ~/.bashrc

## Usage:
`CLMYarp --from $CLM_MODEL_DIR`


Note: A bunch of video files for testing the code and the main source-code of CLM can be found at:
https://github.com/TadasBaltrusaitis/CLM-framework


