import os
import unittest
import cv2
import yarp
from yarpRTMPose.inference import RTMPose

current_path = os.path.dirname(__file__)

yarp.Network.init()
yarp.Network.setLocalMode(True)

# Too slow to be a unit test
class RTMPoseInferenceTest(unittest.TestCase):

    def test_keypoints(self):
        img = cv2.imread(os.path.join(current_path,'../../data/000000000785.jpg'))
        det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano"
        pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l"
        
        if not os.path.exists(det_model_path):
            try:
                det_model_path = os.listdir("/mmdeploy/rtmpose-ort/")[0]
            except:
                print("No model has been already deployed to onnxruntime.")


        dataset = "COCO_wholebody"
        device = "cuda"
    
        inferencer = RTMPose(det_model_path,
                            pose_model_path,
                            dataset,device)
        keypoints = inferencer.inference(img)
        keypoints_op_format = inferencer.format_keypoints(keypoints,openpose_format=True)
        keypoints_op_format = keypoints_op_format[0] #we know we have just one person in test image 
        self.assertTrue('LHip' in keypoints_op_format.keys())
        self.assertTrue(3,len(keypoints_op_format['LHip']))
        self.assertTrue('MidHip' in keypoints_op_format.keys())
        self.assertTrue('neck' in keypoints_op_format.keys())

        keypoints_orig_format = inferencer.format_keypoints(keypoints)
        keypoints_orig_format = keypoints_orig_format[0]
        self.assertFalse('LHip' in keypoints_orig_format.keys())
        self.assertTrue('left_hip' in keypoints_orig_format.keys())
        self.assertFalse('MidHip' in keypoints_orig_format.keys())

    def test_keypoints_wholebody(self):
        # img = cv2.imread(os.path.join(current_path,'data/000000000785.jpg'))
        # det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano"
        # pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-m"
        pass

if __name__ == "__main__":
    unittest.main()
