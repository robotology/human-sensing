#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/sig/Image.h>


class yarpRTMPose : public yarp::os::RFModule
{
    private:
        yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>> inport;
        yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>> outport;
}