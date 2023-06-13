#include <string>
#include <yarpRTMPose.h>
#include <filesystem>
#include <iostream>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Keypoints from image","[yarpRTMPose]")
{
    const std::string current_path = std::filesystem::current_path();
    const std::string img_path = current_path + "/../../data/000000000785.jpg";
    cv::Mat test_img = cv::imread(img_path);

    if(test_img.empty())
    {
        std::cout << "Could not read the image" << img_path;
    }

    const std::string det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano";
    const std::string pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l";

    const std::string dataset = "COCO_wholebody"; 
    const std::string device = "cuda";

    RTMPose inferencer{det_model_path,pose_model_path,dataset,device};
    auto [bboxes, keypoints] = inferencer.inference(test_img);

    // There is 1 person in the image
    REQUIRE(keypoints.size() == bboxes.size());

    //COCO_wholebody has 133 kp
    REQUIRE(133 == keypoints[0].length);

} 

// TEST_CASE(ProcessImage,CheckOPKeypoints)
// {
//     //TODO

//     // 1. Build keypoints vector
//     // const std::string current_path = std::filesystem::current_path();
//     // const std::string img_path = current_path + "../../data/000000000785.jpg";
//     // cv::Mat test_img = cv::imread(img_path);

//     // if(test_img.empty())
//     // {
//     //     std::cout << "Could not read the image" << img_path;
//     // }

//     // const std::string det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano";
//     // const std::string pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l";

//     // const std::string dataset = "COCO_wholebody"; 
//     // const std::string device = "cuda";

//     // RTMPose inferencer{det_model_path,pose_model_path,dataset,device};
//     // auto [bboxes, keypoints] = inferencer.inference(test_img);

//     // // 2. apply formatter
//     // inferencer.format_keypoints(keypoints);

//     // 3. check results
// }