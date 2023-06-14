#include <yarp/cv/Cv.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarpRTMPose.h>
#include "mmdeploy/detector.hpp"
#include "utils/visualize.h"
#include <fstream>

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

    if(dataset == "coco_wholebody")
    {
        v.set_skeleton(utils::Skeleton::get("coco-wholebody"));
    }
    else if(dataset == "coco")
    {
        v.set_skeleton(utils::Skeleton::get("coco"));   
    }
    else
    {
        yWarning() << "Cannot paint the given dataset.";
        return cv::Mat();
    }

    auto sess = v.get_session(img);

    for (size_t i = 0; i < bboxes.size(); ++i) {
        sess.add_bbox(bboxes[i], -1, -1);
        sess.add_pose(poses[i].point, poses[i].score, poses[i].length, this->det_thr);
    }

    return sess.get();

}

bool yarpRTMPose::configure(yarp::os::ResourceFinder& rf)
{
    this->period = rf.check("period",yarp::os::Value(0.01)).asFloat32();
    this->det_model_path = rf.find("det_model_path").asString();
    this->pose_model_path = rf.find("pose_model_path").asString();
    this->dataset = rf.check("dataset",yarp::os::Value("coco_wholebody")).asString();
    this->device = rf.check("device",yarp::os::Value("cuda")).asString();
    this->module_name = rf.check("module_name",yarp::os::Value("yarpRTMPose")).asString();

    std::string dataset_file = rf.findFile(this->dataset + ".json");
    // Create the inference engine
    this->inferencer = std::make_unique<RTMPose>(this->det_model_path,this->pose_model_path,this->dataset,this->device);

    // Open yarp ports
    inPort.open("/" + module_name + "/image:i");
    outPort.open("/" + module_name + "/image:o");
    targetPort.open("/" + module_name + "/target:o");

    // Read dataset info json
    if(dataset_file == "")
    {
        yError() << "Dataset file was not found. Either dataset is invalid or the corresponding json file is not installed";
        return -1;
    }
    
    yDebug() << "Dataset file:" << dataset_file;
    std::ifstream r(dataset_file);
    keypointInfo = json::parse(r);

    return true;
}

bool yarpRTMPose::updateModule()
{
    if(auto* frame = inPort.read())
    {
        const cv::Mat img = yarp::cv::toCvMat(*frame);
        //TODO: convert frame from yarp to cv
        auto [bboxes, keypoints] = inferencer->inference(img);

        //Convert keypoints to bottle format
        yarp::os::Bottle& target = targetPort.prepare();
        target.addList().read(this->kpToBottle(keypoints));

        cv::Mat keypoints_img = inferencer->paint(img,bboxes,keypoints);
        
        auto& outImage = outPort.prepare();
        outImage = yarp::cv::fromCvMat<yarp::sig::PixelRgb>(keypoints_img);
        outPort.write();

    }

    return true;
}

bool yarpRTMPose::interruptModule()
{
    return close();
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
    yarp::os::Bottle targetBottle;

    for(auto &skeleton: keypoints)
    {
        yarp::os::Bottle skeletonBottle;

        //TODO: right now it is not compatible with openpose face
        for(size_t i=0; i<skeleton.length; ++i)
        {
            yarp::os::Bottle keypointBottle;

            const auto &kp = skeleton.point[i];
            keypointBottle.addString(this->keypointInfo[std::to_string(i)]["name"]);
            keypointBottle.addFloat32(kp.x);
            keypointBottle.addFloat32(kp.y);

            skeletonBottle.addList().read(keypointBottle);
        }

        targetBottle.addList().read(skeletonBottle);
    }

    return targetBottle;
}