#include <string>
#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/sig/Image.h>
#include <opencv2/opencv.hpp>
#include "mmdeploy/detector.hpp"
#include "mmdeploy/pose_detector.hpp"

using RgbImagePort = yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>>;

class RTMPose
{
    public:
        RTMPose(std::string det_model_path,
                std::string pose_model_path,
                std::string dataset,
                std::string device);

        std::pair<std::vector<mmdeploy_rect_t>,mmdeploy::cxx::PoseDetector::Result> inference(const cv::Mat& img);
        cv::Mat paint(const cv::Mat& img, 
                            const std::vector<mmdeploy_rect_t> &bboxes,
                            const mmdeploy::PoseDetector::Result poses);
        

        mmdeploy::Detector detector;
        mmdeploy::PoseDetector pose_model;
        int det_label;
        double det_thr;
        std::string dataset;
        std::string device;

};

class yarpRTMPose : public yarp::os::RFModule
{
    private:

        int height;
        int width;
        double period;
        std::string module_name;
        std::string device;
        std::string dataset;
        std::string det_model_path;
        std::string pose_model_path;
        bool openpose_format;

        RgbImagePort inPort;
        RgbImagePort outPort;
        yarp::os::BufferedPort<yarp::os::Bottle> targetPort;

    public:
        std::unique_ptr<RTMPose> inferencer;
        bool configure(yarp::os::ResourceFinder& rf) override;
        bool updateModule() override;
        bool close() override;
        double getPeriod() override;
        yarp::os::Bottle kpToBottle(const mmdeploy::cxx::PoseDetector::Result& keypoints);

};
