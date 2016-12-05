# human-sensing
This repository contains software related to human sensing.
It is currently under major overhauling and all previous software + documentation has been kept in the ***under-review*** branch and will soon be transferred to ***master***. in the meanwhile, if parts of the code is required please use the following git command if you already have a version pf the repo: ```git checkout under-review``` otherwise directly clone the under-review branch: ```git clone -b under-review https://github.com/robotology/human-sensing.git```

##Progress on repository restructuring
Project updates can be found [here](https://github.com/robotology/human-sensing/projects/1)

## Installation

#### Dependencies
In order to compile and run modules in this repository you will need to install the following dependencies.
Installation are standards and instructions can be found in the following links. Specific installation for ```dlib``` can be found below.
- [YARP](https://github.com/robotology/yarp)
- [iCub](https://github.com/robotology/icub-main)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html)
-  dlib

#### dlib Installation
On macOS:

    brew install homebrew/science/dlib
    brew install homebrew/dupes/bzip2

On Linux (Xenial 16.04):

    sudo apt-get install libdlib-dev
    sudo apt-get install bzip2

On Linux < 16.04 or Windows:

    download dlib from http://dlib.net/
    generate cmake and make install

#### Compilation

While in the root folder of ```human-sensing``` do:

    mkdir build
    cd build
    ccmake ..

##### side note
Some modules require the ```shape_predictor_68_face_landmarks.dat``` file to correctly run.
As the file is >60MB it is not included in the repository but if the user sets the ```DOWNLOAD_FACE_LANDMARKS_DAT``` to ```ON```.
Cmake will automatically take care of downloading the required file, unzip it and install it correctly.
If you prefer to do it on your own, please download the ```.dat``` file from [here](http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2), unzip it and copy it to the ```build``` directory.

    configure & generate
    make install

## Documentation
Online documentation is available here: [http://robotology.github.com/human-sensing](http://robotology.github.com/human-sensing).

## License

Material included here is Copyright of _iCub Facility - Istituto Italiano di Tecnologia_ and is released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
