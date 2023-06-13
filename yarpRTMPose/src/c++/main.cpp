#include <yarpRTMPose.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>

int main(int argc, char *argv[])
{
    yarp::os::Network::init();

    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return 1;
    }

    yarpRTMPose module;
    yarp::os::ResourceFinder rf;

    rf.setVerbose( true );
    rf.setDefaultContext( "yarpOpenPose" );
    rf.setDefaultConfigFile( "yarpOpenPose.ini" );
    rf.setDefault("name","yarpOpenPose");
    rf.configure(argc,argv);

    return module.runModule(rf);
}
