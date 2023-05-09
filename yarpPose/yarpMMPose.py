from yarpPose import yarpPose
from mmpose.apis import MMPoseInferencer
import importlib.util
import sys
import time

class yarpMMPose(yarpPose):

    # def __init__(self,poseInferencer,dataset='COCO'):
    #     self.inferencer = poseInferencer
    #     self.dataset = dataset
    #     super().__init__("MMPose")

    # @classmethod
    # def fromalias(cls,alias='human',dataset='COCO'):
    #     return cls(MMPoseInferencer(alias),dataset)

    
    def __init__(self,config,model,det_model=None,det_weights=None,dataset='COCO'):
        if det_model and det_weights:
            print(det_weights)
            inferencer = MMPoseInferencer(config,model,det_model=det_model,det_weights=det_weights)
        else:
            inferencer = MMPoseInferencer(config,model)

        self.inferencer = inferencer
        self.dataset = dataset

        if self.dataset == 'COCO':
            spec = importlib.util.spec_from_file_location("coco",'/mmpose/configs/_base_/datasets/coco.py')
            dataset_module = importlib.util.module_from_spec(spec)
            sys.modules["coco"] = dataset_module
            spec.loader.exec_module(dataset_module)
        
        self.keypoint_info = dataset_module.dataset_info["keypoint_info"]    
            
        super().__init__("yarpMMPose")

    @classmethod
    def fromconfig(cls,rf):
        config = rf.find('config').asString()
        model = rf.find('model').asString()
        if rf.check('det_model'):
            det_model = rf.find('det_model').asString()
        else:
            det_model = None
        if rf.check('det_weights'):
            det_weights = rf.find('det_weights').asString()
        else:
            det_weights = None
        if rf.check('dataset'):
            dataset = rf.find('dataset').asString()
        else:
            dataset = 'COCO'

        return cls(config,model,det_model,det_weights,dataset)

    def inference(self,img):

        generator = self.inferencer(img)
        start = time.time()
        result = next(generator)
        end = time.time()
        print("inf time:", end-start)

        keypoints = [] 
        predictions = result["predictions"]

        for person in predictions[0]: 
            person_keypoints = {} 
            for i, keypoint in enumerate(person["keypoints"]):
                person_keypoints[self.keypoint_info[i]["name"]] = keypoint 
                person_keypoints[self.keypoint_info[i]["name"]].append(person["keypoint_scores"][i])

            keypoints.append(person_keypoints)
        
        return keypoints
