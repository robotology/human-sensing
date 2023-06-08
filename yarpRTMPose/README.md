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

app_test_data_player assumes that you have a yarpdataplayer streaming a rgbimage on the port /cer/realsense_repeater/rgbImage:o. Else you need to modify either the name of the port in the application xml file or in the dataplayer itself.

#### Using a video file as an input

**Only .avi files are supported**

In order to use the app_test_video_grabber application you have to perform the following actions:
- Launch `start_docker.sh DATA_DIR IMG_NAME` where DATA_DIR is the directory in which you keep the video you wish to analyze and IMG_NAME is the name of the docker image you built 
- Remember to modify the config file `grabber.ini` with the path to the video you wish to replay and `yarpRTMPose.ini` with the resolution of your video. 

## What's to come

- Wholebody skeleton drawing
- C++ application
- Pose tracking (? yet to be checked if feasible within the RTMPose library)