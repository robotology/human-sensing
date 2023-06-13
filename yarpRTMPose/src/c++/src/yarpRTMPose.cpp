#include <string>
#include <yarpRTMPose.h>
#include "mmdeploy/detector.hpp"
#include "utils/visualize.h"

RTMPose::RTMPose(std::string det_model_path,
                    std::string pose_model_path,
                    std::string dataset,
                    std::string device):
                    detector{mmdeploy::Model(det_model_path),mmdeploy::Device(device)},
                    pose_model{mmdeploy::Model(pose_model_path),mmdeploy::Device(device)},
                    dataset{dataset},
                    device{device},
                    det_label{0},
                    det_thr{0.5}
                    {}

std::pair<std::vector<mmdeploy_rect_t>,mmdeploy::cxx::PoseDetector::Result> RTMPose::inference(const cv::Mat &img)
{
    mmdeploy::Detector::Result dets = detector.Apply(img);
    std::vector<mmdeploy_rect_t> bboxes;
    for (const auto& det:dets)
    {
        if (det.label_id == this->det_label && det.score > this->det_thr) {
           bboxes.push_back(det.bbox);
        }
    }

    // apply pose detector, if no bboxes are provided, full image will be used.
    mmdeploy::PoseDetector::Result poses = pose_model.Apply(img, bboxes);

    return std::make_pair(bboxes,poses);
}


cv::Mat RTMPose::paint(const cv::Mat& img, 
                const std::vector<mmdeploy_rect_t> &bboxes, 
                const mmdeploy::PoseDetector::Result poses)
{
    
    utils::Visualize v;
    v.set_skeleton(utils::Skeleton::get(dataset));

    auto sess = v.get_session(img);

    for (size_t i = 0; i < bboxes.size(); ++i) {
        sess.add_bbox(bboxes[i], -1, -1);
        sess.add_pose(poses[i].point, poses[i].score, poses[i].length, this->det_thr);
    }

    return sess.get();

}

bool yarpRTMPose::configure(yarp::os::ResourceFinder& rf)
{
    // Reading configuration from resource finder
    this->width = rf.check("width",yarp::os::Value(1280)).asInt32();
    this->height = rf.check("height",yarp::os::Value(720)).asInt32();
    this->period = rf.check("period",yarp::os::Value(0.01)).asFloat32();
    this->det_model_path = rf.find("det_model_path").asString();
    this->pose_model_path = rf.find("pose_model_path").asString();
    this->dataset = rf.check("dataset",yarp::os::Value("COCO_wholebody")).asString();
    this->device = rf.check("device",yarp::os::Value("cuda")).asString();
    this->module_name = rf.check("module_name",yarp::os::Value("yarpRTMPose")).asString();

    // Create the inference engine
    this->inferencer = std::make_unique<RTMPose>(this->det_model_path,this->pose_model_path,this->dataset,this->device);

    // Open yarp ports
    inPort.open("/" + module_name + "/image:i");
    outPort.open("/" + module_name + "/image:o");
    targetPort.open("/" + module_name + "/target:o");

    return true;
}

bool yarpRTMPose::updateModule()
{
    // if(auto frame = inPort.read())
    // {
    //     //TODO: convert frame from yarp to cv
    //     auto [bboxes, keypoints] = this->inferencer->inference(frame);

    //     //Convert keypoints to bottle format

    //     yarp::os::Bottle& target = targetPort.prepare();
    //     target.addList().read(this->kpToBottle(keypoints));


    // }

    return true;
}

bool yarpRTMPose::close()
{
    this->inPort.close();
    this->outPort.close();
    this->targetPort.close();
    return true;
}

double yarpRTMPose::getPeriod()
{
    return period;
}

yarp::os::Bottle yarpRTMPose::kpToBottle(const mmdeploy::cxx::PoseDetector::Result& keypoints)
{

    // yarp::os::Bottle targetBottle;

    // for(auto &skeleton: keypoints)
    // {
    //     yarp::os::Bottle skeletonBottle;

    //     for(size_t i=0; i<skeleton.length; ++i)
    //     {
    //         yarp::os::Bottle keypointBottle;

    //         const auto &kp = skeleton.point[i];
    //         targetBottle.addString(this->keypoint_mapping[i]);
    //         targetBottle.addFloat32(kp.x);
    //         targetBottle.addFloat32(kp.y);
    //     }
    // }
}