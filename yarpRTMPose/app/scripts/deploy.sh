#!/bin/bash

# go to the mmdeploy folder
cd /mmdeploy

# Check if there are already deployed models locally and copy them in the expected path
DEPLOYED_MODELS_DIR="/yarpRTMPose/download/deployed_models"
DEPLOYED_MODELS=$(ls ${DEPLOYED_MODELS_DIR} 2>/dev/null)
echo $DEPLOYED_MODELS
if [ -n "${DEPLOYED_MODELS}" ]; then
	echo "hello there?"
	mv ${DEPLOYED_MODELS_DIR}/${DEPLOYED_MODELS} .
else
	# Model file can be either a local path or a download link
	MODELS_DIR="/yarpRTMPose/download/models/rtmpose"

	DET_FILE="rtmdet_nano_8xb32-100e_coco-obj365-person-05d8511e.pth"
	WHOLEBODY_L_POSE_FILE="rtmpose-l_simcc-coco-wholebody_pt-aic-coco_270e-256x192-6f206314_20230124.pth"
	# Right now the two following files are not used
	# WHOLEBODY_M_POSE_FILE="rtmpose-m_simcc-coco-wholebody_pt-aic-coco_270e-256x192-cd5e845c_20230123.pth"
	# BODY_M_POSE_FILE="rtmpose-m_simcc-aic-coco_pt-aic-coco_420e-256x192-63eb25f7_20230126.pth"

	if [ -f "$MODELS_DIR/det/$DET_FILE" ]
	then
	FILE="$MODELS_DIR/det/$DET_FILE"
	else
	FILE="https://download.openmmlab.com/mmpose/v1/projects/rtmpose/$DET_FILE"
	fi

	python tools/deploy.py \
			configs/mmdet/detection/detection_onnxruntime_static.py \
			../mmpose/projects/rtmpose/rtmdet/person/rtmdet_nano_320-8xb32_coco-person.py \
			$FILE \
			demo/resources/human-pose.jpg \
			--work-dir rtmpose-ort/rtmdet-nano \
			--device cpu \
			--dump-info  # dump sdk info

	if [ -f "$MODELS_DIR/pose/$WHOLEBODY_L_POSE_FILE" ]
	then
	FILE="$MODELS_DIR/pose/$WHOLEBODY_L_POSE_FILE"
	else
	FILE="https://download.openmmlab.com/mmpose/v1/projects/rtmpose/${WHOLEBODY_L_POSE_FILE}"
	fi


	python tools/deploy.py \
			configs/mmpose/pose-detection_simcc_onnxruntime_dynamic.py \
			../mmpose/projects/rtmpose/rtmpose/wholebody_2d_keypoint/rtmpose-l_8xb64-270e_coco-wholebody-256x192.py \
			$FILE \
			demo/resources/human-pose.jpg \
			--work-dir rtmpose-ort/rtmpose-l \
			--device cpu \
			--dump-info  # dump sdk info
fi

