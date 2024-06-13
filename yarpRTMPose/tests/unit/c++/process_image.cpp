#include <string>
#include <yarpRTMPose.h>
#include <filesystem>
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>

TEST_CASE("Keypoints from image","[yarpRTMPose]")
{
    const std::filesystem::path img_path{"../../tests/data/000000000785.jpg"};

    cv::Mat test_img{cv::imread(img_path)};

    if(test_img.empty())
    {
        std::cout << "Could not read the image" << img_path << std::endl;
    }

    const std::filesystem::path det_model_path{"../../download/deployed_models/rtmpose-ort/rtmdet-nano"};
    const std::filesystem::path pose_model_path{"../../download/deployed_models/rtmpose-ort/rtmpose-l"};

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
    const std::filesystem::path img_path{"../../tests/data/000000000785.jpg"};
    cv::Mat test_img = cv::imread(img_path);

    if(test_img.empty())
    {
        std::cout << "Could not read the image" << img_path;
    }

    const std::filesystem::path det_model_path{"../../download/deployed_models/rtmpose-ort/rtmdet-nano"};
    const std::filesystem::path pose_model_path{"../../download/deployed_models/rtmpose-ort/rtmpose-l"};

    const std::string dataset = "coco_wholebody"; 
    const std::string device = "cuda";

    RTMPose inferencer{det_model_path,pose_model_path,dataset,device};
    auto [bboxes, keypoints] = inferencer.inference(test_img);

    // 2. paint the results
    cv::Mat kp_img = inferencer.paint(test_img,bboxes,keypoints);

    // 3. check results
    cv::imwrite("/test.png",kp_img);
}