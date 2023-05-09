#!/bin/bash

# go to the mmdeploy folder
cd /mmdeploy

# USE THE FOLLOWING TWO FOR BODY ONLY
# run the command to convert RTMDet
# Model file can be either a local path or a download link
# python tools/deploy.py \
#     configs/mmdet/detection/detection_onnxruntime_static.py \
#     ../mmpose/projects/rtmpose/rtmdet/person/rtmdet_nano_320-8xb32_coco-person.py \
#     https://download.openmmlab.com/mmpose/v1/projects/rtmpose/rtmdet_nano_8xb32-100e_coco-obj365-person-05d8511e.pth \
#     demo/resources/human-pose.jpg \
#     --work-dir rtmpose-ort/rtmdet-nano \
#     --device cpu \
#     --dump-info  # dump sdk info

# # run the command to convert RTMPose
# # Model file can be either a local path or a download link
# python tools/deploy.py \
#     configs/mmpose/pose-detection_simcc_onnxruntime_dynamic.py \
#     ../mmpose/projects/rtmpose/rtmpose/body_2d_keypoint/rtmpose-m_8xb256-420e_coco-256x192.py \
#     https://download.openmmlab.com/mmpose/v1/projects/rtmposev1/rtmpose-m_simcc-aic-coco_pt-aic-coco_420e-256x192-63eb25f7_20230126.pth \
#     demo/resources/human-pose.jpg \
#     --work-dir rtmpose-ort/rtmpose-m \
#     --device cpu \
#     --dump-info  # dump sdk info


# run the command to convert RTMDet
# Model file can be either a local path or a download link

FILE="/yarpRTMPose/download/models/rtmpose/det/rtmdet_nano_8xb32-100e_coco-obj365-person-05d8511e.pth"
if [ -f "$FILE" ]
then
python tools/deploy.py \
    configs/mmdet/detection/detection_onnxruntime_static.py \
    ../mmpose/projects/rtmpose/rtmdet/person/rtmdet_nano_320-8xb32_coco-person.py \
    $FILE \
    demo/resources/human-pose.jpg \
    --work-dir rtmpose-ort/rtmdet-nano \
    --device cpu \
    --dump-info  # dump sdk info
else
python tools/deploy.py \
    configs/mmdet/detection/detection_onnxruntime_static.py \
    ../mmpose/projects/rtmpose/rtmdet/person/rtmdet_nano_320-8xb32_coco-person.py \
    https://download.openmmlab.com/mmpose/v1/projects/rtmpose/rtmdet_nano_8xb32-100e_coco-obj365-person-05d8511e.pth \
    demo/resources/human-pose.jpg \
    --work-dir rtmpose-ort/rtmdet-nano \
    --device cpu \
    --dump-info  # dump sdk info
fi

# run the command to convert RTMPose
# Model file can be either a local path or a download link
FILE="/yarpRTMPose/download/models/rtmpose/pose/rtmpose-l_simcc-coco-wholebody_pt-aic-coco_270e-256x192-6f206314_20230124.pth"
if [ -f "$FILE" ]
then
python tools/deploy.py \
    configs/mmpose/pose-detection_simcc_onnxruntime_dynamic.py \
    ../mmpose/projects/rtmpose/rtmpose/wholebody_2d_keypoint/rtmpose-l_8xb64-270e_coco-wholebody-256x192.py \
    $FILE \
    demo/resources/human-pose.jpg \
    --work-dir rtmpose-ort/rtmpose-l \
    --device cpu \
    --dump-info  # dump sdk info
else
python tools/deploy.py \
    configs/mmpose/pose-detection_simcc_onnxruntime_dynamic.py \
    ../mmpose/projects/rtmpose/rtmpose/body_2d_keypoint/rtmpose-m_8xb256-420e_coco-256x192.py \
    https://download.openmmlab.com/mmpose/v1/projects/rtmposev1/rtmpose-m_simcc-aic-coco_pt-aic-coco_420e-256x192-63eb25f7_20230126.pth \
    demo/resources/human-pose.jpg \
    --work-dir rtmpose-ort/rtmpose-m \
    --device cpu \
    --dump-info  # dump sdk info
fi