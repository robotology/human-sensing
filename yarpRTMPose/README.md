# yarpRTMPose

A 2d human pose estimation yarp module powered by **RTMPose**. It's (much) faster and more precise than yarpopenpose, while being less GPU intensive. For more details check the amazingly informative readme by the open-mmlab project https://github.com/open-mmlab/mmpose/tree/main/projects/rtmpose.

## Installation

### Prerequisites

- Docker
- A machine equipped with a nvidia gpu

### Procedure

You can download the whole human sensing repository or only this subdirectory. For conveniency reason we only provide instructions to build the docker image.  

```sh
git clone https://github.com/fbrand-new/human-sensing.git
cd human-sensing/yarpRTMPose
./docker/build_docker <my_img_name> #Equivalent of docker build -t <my_img_name> . 
```

## Usage

### Testing yarpRTMPose

#### Using Yarpdataplayer as a video input

app\_test\_data\_player assumes that you have a yarpdataplayer streaming a rgbimage on the port /cer/realsense\_repeater/rgbImage:o. Else you need to modify either the name of the port in the application xml file or in the dataplayer itself.

#### Using a video file as an input

**Only .avi files are supported**

In order to use the app\_test\_video\_grabber application you have to perform the following actions:
- Launch `start_docker.sh DATA_DIR IMG_NAME` where DATA\_DIR is the directory in which you keep the video you wish to analyze and IMG\_NAME is the name of the docker image you built 
- Remember to modify the config file `grabber.ini` with the path to the video you wish to replay and `yarpRTMPose.ini` with the resolution of your video. 

## What's to come

- Wholebody skeleton drawing
- C++ application
- Pose tracking (? yet to be checked if feasible within the RTMPose library)

## Misc

### Speeding up docker build

There are a couple of utility scripts that are made to permanently download some files which are used in the docker image. This comes in handy if you have to rebuild the image continuously, since the build process will download from scratch the same files all over again.

- `app/scripts/download_models.sh` will download models checkpoint (detector and wholebody large pose estimation) into `downloads/models`.
- `app/scripts/copy_deployed_models.sh` will copy the onnxruntime models deployed my mmdeploy from inside an active docker container. Check the help of the script for further information.
