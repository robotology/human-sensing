import os
import unittest
import cv2
from yarpRTMPose.yarpRTMPose import RTMPose

current_path = os.path.dirname(__file__)

# Too slow to be a unit test
class RTMPoseInferenceTest(unittest.TestCase):

    def test_keypoints(self):
        img = cv2.imread(os.path.join(current_path,'data/000000000785.jpg'))
        det_model_path = "/mmdeploy/rtmpose-ort/rtmdet-nano"
        pose_model_path = "/mmdeploy/rtmpose-ort/rtmpose-l"
        dataset = "COCO_wholebody"
        device = "cuda"

        inferencer = RTMPose(det_model_path,pose_model_path,dataset,device)
        keypoints = inferencer.inference(img)
        keypoints = inferencer.format_keypoints(keypoints)

        #These next lines can vary wildly depending on the framework. We need to abstract this
        keypoints = keypoints[0] #we know we have just one person in test image 
        print(keypoints)
        self.assertTrue('left_hip' in keypoints.keys())
        self.assertTrue(3,len(keypoints['left_hip']))

if __name__ == "__main__":
    unittest.main()
