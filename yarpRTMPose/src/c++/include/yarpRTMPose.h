#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/Image.h>
#include <opencv2/opencv.hpp>
#include "mmdeploy/detector.hpp"
#include "mmdeploy/pose_detector.hpp"
#include <nlohmann/json.hpp>

using RgbImagePort = yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb>>;
using json = nlohmann::json;

class RTMPose
{
public:
    RTMPose(std::string det_model_path,
            std::string pose_model_path,
            std::string dataset,
            std::string device);

    std::pair<std::vector<mmdeploy_rect_t>, mmdeploy::cxx::PoseDetector::Result> inference(const cv::Mat &img);
    cv::Mat paint(const cv::Mat &img,
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

    json keypointInfo;

    std::size_t face_keypoint_idx_start;
    std::size_t face_keypoint_idx_end;

    json op_not_in_coco_json; // optional json used only if openpose format is true

public:
    std::unique_ptr<RTMPose> inferencer;

    bool configure(yarp::os::ResourceFinder &rf) override;
    bool updateModule() override;
    bool interruptModule() override;
    bool close() override;
    double getPeriod() override;
    yarp::os::Bottle kpToBottle(const mmdeploy::cxx::PoseDetector::Result &keypoints);
    std::pair<size_t, size_t> faceKeypointsIdxs();
};
