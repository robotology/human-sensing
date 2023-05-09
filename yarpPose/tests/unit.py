import os
import unittest
import cv2
from yarpMMPose import yarpMMPose

current_path = os.path.dirname(__file__)

# Too slow to be a unit test
class MMPoseInferenceTest(unittest.TestCase):

    def test_keypoints(self):
        img = cv2.imread(os.path.join(current_path,'data/000000000785.jpg'))
        #inferencer = yarpMMPose.fromalias('human')
        config_file = '/mmpose/configs/body_2d_keypoint/topdown_heatmap/coco/' \
           'td-hm_hrnet-w32_8xb64-210e_coco-256x192.py'
        weights_file = '/mmpose/models/top_down/hrnet/hrnet_w32_coco_256x192-c78dce93_20200708.pth'
        
        print(config_file)
        inferencer = yarpMMPose(config_file,weights_file)
        keypoints = inferencer.inference(img)

        #These next lines can vary wildly depending on the framework. We need a to abstract this
        keypoints = keypoints[0] #we know we have just one person in test image 
        self.assertTrue('left_hip' in keypoints.keys())
        self.assertTrue(3,len(keypoints['left_hip']))

if __name__ == "__main__":
    unittest.main()
