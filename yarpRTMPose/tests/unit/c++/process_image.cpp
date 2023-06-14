#include <string>
#include <yarpRTMPose.h>
#include <filesystem>
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>

TEST_CASE("Keypoints from image","[yarpRTMPose]")
{
    std::filesystem::path img_path("/yarpRTMPose/tests/data/000000000785.jpg");
    cv::Mat test_img = cv::imread(img_path);

    if(test_img.empty())
    {
        std::cout << "Could not read the image" << img_path;
    }

    const std::string det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano";
    const std::string pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l";

    const std::string dataset = "coco_wholebody"; 
    const std::string device = "cuda";

    RTMPose inferencer{det_model_path,pose_model_path,dataset,device};
    auto [bboxes, keypoints] = inferencer.inference(test_img);

    // There is 1 person in the image
    REQUIRE(keypoints.size() == bboxes.size());

    //COCO_wholebody has 133 kp
    REQUIRE(133 == keypoints[0].length);

} 

TEST_CASE("RTMPose pipeline","[yarpRTMPose]")
{
    //TODO

    // 1. Build keypoints vector
    std::filesystem::path img_path("/yarpRTMPose/tests/data/000000000785.jpg");
    cv::Mat test_img = cv::imread(img_path);

    if(test_img.empty())
    {
        std::cout << "Could not read the image" << img_path;
    }

    const std::string det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano";
    const std::string pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l";

    const std::string dataset = "coco_wholebody"; 
    const std::string device = "cuda";

    RTMPose inferencer{det_model_path,pose_model_path,dataset,device};
    auto [bboxes, keypoints] = inferencer.inference(test_img);

    yDebug() << "Inference working";
    // 2. paint the results
    cv::Mat kp_img = inferencer.paint(test_img,bboxes,keypoints);

    yDebug() << "paint not working";
    // 3. check results
    cv::imwrite("/test.png",kp_img);
}