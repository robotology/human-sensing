# faceLandmarks dependencies installation
This readme contains information on how to build and install all required dependencies for the faceLandmarks module.

Table of Contents
=================
* [Required Dependencies](#generic_dep)
* [dlib Installation](#dlibinstallation)
* [Compilation](#compilation)
* [Side note](#sidenote)

#### Required Dependencies

- [YARP](https://github.com/robotology/yarp)
- [iCub](https://github.com/robotology/icub-main)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html)
- [dlib](http://dlib.net)

#### dlib Installation
On macOS:

    brew install homebrew/science/dlib (this installs version 19.2)
    brew install homebrew/dupes/bzip2

On Linux (Xenial 16.04):

    sudo apt-get update
    sudo apt-get install bzip2 libpng-dev libjpeg-dev libblas-dev liblapack-dev libsqlite3-dev
    sudo apt-get install libdlib-dev (this installs version 18.18-1, if a more recent version is required please download manually as described below)

I have just noted that there is a bug in the dlib package:

    CMake Error at /usr/lib/cmake/dlib/dlib.cmake:76 (message):
    The imported target "dlib::dlib" references the file
     "/usr/lib/libdlib.a"
     ...

CMake looks for the static library ```libdlib.a``` while the package installs the dynamic one instead ```libdlib.so```.
Unfortunately until the developers update their package it is required to run the following command:

    sudo sed -i '17s/^/#/' /usr/lib/cmake/dlib/dlib-none.cmake
This command actually comments out the line that checks for the ```libdlib.a``` file, and you are good to go.

On Linux < 16.04 or dlib version > 18.18-1 or Windows (more details soon):

    download dlib from http://dlib.net/
    generate cmake and make install

#### Compilation

While in the root folder of ```human-sensing``` do:

    mkdir build
    cd build
    ccmake ..

##### Side note
Some modules require the ```shape_predictor_68_face_landmarks.dat``` file to correctly run.
As the file is >60MB it is not included in the repository but if the user sets the ```DOWNLOAD_FACE_LANDMARKS_DAT``` to ```ON```.
Cmake will automatically take care of downloading the required file, unzip it and install it correctly.
If you prefer to do it on your own, please download the ```.dat``` file from [here](http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2), unzip it and copy it to the ```build``` directory.

    configure & generate
    make install
