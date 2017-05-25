# human-sensing
This repository contains software related to human sensing

The code has been tested on Ubuntu 14.04 LTS & Lubuntu 14.04 LTS

The code needs the follwoing libraries:

    1- BLAS (for dlib c++ library)
    2- OpenCV 3.0
    2- tbb
    3- boost
    4- dlib (for face detection)
    5- icub-main (implemented on icub)

## Get BLAS:

    sudo apt-get install libopenblas-dev liblapack-dev 

## Get OpenCV
1- Install the dependencies:

    sudo apt-get install git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
    sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev   libjasper-dev libdc1394-22-dev checkinstall
  
2- Download the source code: 

    https://github.com/Itseez/opencv/archive/3.0.0-beta.zip

3- build and install:

    cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=ON -D BUILD_SHARED_LIBS=OFF ..
    make -j2
    sudo make install	

## Get TBB:

    sudo apt-get install libtbb-dev

## Get Boost:

    sudo apt-get install libboost-all-dev
    
## Get dlib:

Download the library (v18.xx) from the following link and install it:

    http://dlib.net/ 


finally, build the main code and test it by running the `SimpleCLM` executable. 
  
## Get iCub-main:

clone from the following link,

    https://github.com/robotology/icub-main

## Clone and build this repository

Add `CLM_MODEL_DIR=$SRC_FOLDER/human-sensing-SAM/app/CLM_Yarp/conf` to your ~/.bashrc

## Usage:
CLMYarp --from $CLM_MODEL_DIR


Note: A bunch of video files for testing the code and the main source-code of CLM can be found at:
https://github.com/TadasBaltrusaitis/CLM-framework


