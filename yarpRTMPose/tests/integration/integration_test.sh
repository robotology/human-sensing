# Run me with ./start_docker.sh -c /yarpRTMPose/tests/integration/integration_test.sh

yarpserver &
yarp wait /root
yarprun --server /container-rtmpose --log &
yarp wait /container-rtmpose
yarpmanager-console --application  /yarpRTMPose/app/scripts/app_test_video_grabber_cpp.xml --run --connect