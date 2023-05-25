### Usage

In order to use the app_test_video_grabber application you have two options:
- export YARP_DATA_DIRS=$YARP_DATA_DIRS:\<install-dir\>/human-sensing/yarpRTMPose:\<your-yarp-installation-unless-already-in-yarpdatadirs>
- modify the start_docker.sh with -v \<my-video-dir\>:/\<my-video-dir> and then modify the yarpmanager accordingly.