## Usage

### Build the docker image

You can also use the utility script `docker/build_docker.sh` inputting the image name desired in the format `prefix/image:tag`.

### Testing yarpRTMPose

#### From Yarpdataplayer

app_test_data_player assumes that you have a yarpdataplayer streaming a rgbimage on the port /cer/realsense_repeater/rgbImage:o. Else you need to modify either the name of the port in the application xml file or in the dataplayer itself.

#### From Video

In order to use the app_test_video_grabber application you have to perform the following actions:
- Launch `start_docker.sh DATA_DIR IMG_NAME` where DATA_DIR is the directory in which you keep the video you wish to analyze and IMG_NAME is the name of the docker image you built 
- Remember to modify the config file `grabber.ini` with the path to the video you wish to replay and `yarpRTMPose.ini` with the resolution of your video. 

## What's to come

- Wholebody skeleton drawing
- C++ application
- Pose tracking (? yet to be checked if feasible within the RTMPose library)