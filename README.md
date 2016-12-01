# human-sensing
This repository contains software related to human sensing.
It is currently under major overhauling and all previous software + documentation has been kept in the ***under-review*** branch and will soon be transferred to ***master***. in the meanwhile, if parts of the code is required please use the following git command if you already have a version pf the repo: ```git checkout under-review``` otherwise directly clone the under-review branch: ```git clone -b under-review https://github.com/robotology/human-sensing.git```

## Installation

##### Dependencies
In order to compile and run modules in this repository you will need to install the following dependencies.
Installation are standards and instructions can be found in the following links. Specific installation for ```dlib``` can be found below.
- [YARP](https://github.com/robotology/yarp)
- [iCub](https://github.com/robotology/icub-main)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html)
-  dlib

##### dlib Installation
On macOS:

    brew install homebrew/science/dlib

On macOS:

    sudo apt-get install libdlib-dev

On Windows:

    download dlib @ http://dlib.net/
    generate cmake and make install

## Documentation
Online documentation is available here: [http://robotology.github.com/human-sensing](http://robotology.github.com/human-sensing).

## License

Material included here is Copyright of _iCub Facility - Istituto Italiano di Tecnologia_ and is released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
