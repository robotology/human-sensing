#include <yarpRTMPose.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Log.h>

int main(int argc, char *argv[])
{
    yarp::os::Network::init();

    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        yError() << "YARP server not available!";
        return 1;
    }

    yarpRTMPose module;
    yarp::os::ResourceFinder rf;

    rf.setVerbose(true);
    rf.setDefaultContext("yarpRTMPose");
    rf.setDefaultConfigFile("yarpRTMPose.ini");
    rf.setDefault("name","yarpRTMPose");
    rf.configure(argc,argv);

    return module.runModule(rf);
}
