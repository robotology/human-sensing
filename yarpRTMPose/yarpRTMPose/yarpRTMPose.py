import numpy as np
import yarp
import os
import sys
from pathlib import Path
from importlib import import_module
from importlib.util import spec_from_file_location, module_from_spec 
from collections import defaultdict
from mmdeploy_runtime import Detector, PoseDetector
import cv2

from yarpRTMPose.format_mapping import COCO_WHOLEBODY_TO_OP, OP_TO_COCO_WHOLEBODY

class RTMPose():

    def __init__(self,det_model,pose_model,dataset,device,openpose_format=True):
        self.detector = Detector(
            model_path = det_model,
            device_name = device)
        self.pose_detector = PoseDetector(
            model_path = pose_model,
            device_name = device)

        self.dataset = dataset

        if self.dataset == 'COCO':
            spec = spec_from_file_location("coco",'/mmpose/configs/_base_/datasets/coco.py')
            dataset_module = module_from_spec(spec)
            sys.modules["coco"] = dataset_module
            spec.loader.exec_module(dataset_module)
        
        if self.dataset == 'COCO_wholebody':
            spec = spec_from_file_location("coco_wholebody",'/mmpose/configs/_base_/datasets/coco_wholebody.py')
            dataset_module = module_from_spec(spec)
            sys.modules["coco_wholebody"] = dataset_module
            spec.loader.exec_module(dataset_module)
            
        keypoint_info = dataset_module.dataset_info["keypoint_info"]

        # Keeps yarpopenpose keypoints output format
        self.openpose_format = openpose_format
        if self.openpose_format:
            self.keypoint_info = {}
            for i,keypoints in enumerate(keypoint_info):
                #If I have no explicit mapping replicate the name of the keypoint
                self.keypoint_info[i] = {"name": COCO_WHOLEBODY_TO_OP.get(keypoint_info[i]["name"],
                                                                    keypoint_info[i]["name"])}
        else:
            self.keypoint_info = keypoint_info

    def inference(self,img):
        bboxes, labels, _ = self.detector(img)
        keep = np.logical_and(labels == 0, bboxes[...,4] > 0.6)
        bboxes = bboxes[keep, :4]
        poses = self.pose_detector(img,bboxes)

        return poses

    def format_keypoints(self,poses):

        keypoints = []
        for person in poses:
            person_keypoints = {} 
            for i, keypoint in enumerate(person):
                person_keypoints[self.keypoint_info[i]["name"]] = keypoint 

            keypoints.append(person_keypoints)
        
        if self.openpose_format:
            openpose_keypoints = defaultdict(list)
            for kp in keypoints: #kp is a dict {'kp_name': [x,y,score]}
                for key in kp: #non pythonic
                    if 'face' not in key:
                        openpose_keypoints[key] = kp[key] 
                    else:
                        openpose_keypoints['face'].append(kp[key])

        return keypoints

    #TODO: generalize based on dataset info
    def paint(self,frame,keypoints,thr=0.5,resize=1280):
        skeleton = [(15, 13), (13, 11), (16, 14), (14, 12), (11, 12), (5, 11),
                    (6, 12), (5, 6), (5, 7), (6, 8), (7, 9), (8, 10), (1, 2),
                    (0, 1), (0, 2), (1, 3), (2, 4), (3, 5), (4, 6)]
        palette = [(255, 128, 0), (255, 153, 51), (255, 178, 102), (230, 230, 0),
                   (255, 153, 255), (153, 204, 255), (255, 102, 255),
                   (255, 51, 255), (102, 178, 255),
                   (51, 153, 255), (255, 153, 153), (255, 102, 102), (255, 51, 51),
                   (153, 255, 153), (102, 255, 102), (51, 255, 51), (0, 255, 0),
                   (0, 0, 255), (255, 0, 0), (255, 255, 255)]
        link_color = [
            0, 0, 0, 0, 7, 7, 7, 9, 9, 9, 9, 9, 16, 16, 16, 16, 16, 16, 16
        ]
        point_color = [16, 16, 16, 16, 16, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0]

        scale = resize / max(frame.shape[0], frame.shape[1])

        scores = keypoints[..., 2]
        keypoints = (keypoints[..., :2] * scale).astype(int)

        img = cv2.resize(frame, (0, 0), fx=scale, fy=scale)
        for kpts, score in zip(keypoints, scores):
            show = [0] * len(kpts)
            for (u, v), color in zip(skeleton, link_color):
                if score[u] > thr and score[v] > thr:
                    cv2.line(img, kpts[u], tuple(kpts[v]), palette[color], 1,
                             cv2.LINE_AA)
                    show[u] = show[v] = 1
            for kpt, show, color in zip(kpts, show, point_color):
                if show:
                    cv2.circle(img, kpt, 1, palette[color], 2, cv2.LINE_AA)

        return img

class yarpRTMPose(yarp.RFModule,RTMPose):

    def __init__(self):
        RTMPose.__init__(self)
        yarp.RFModule.__init__(self)

    def configure(self,rf):

        yarp.Network.init()
        
        image_h = rf.find("height").asInt16()
        image_w = rf.find("width").asInt16()

        self.period = 0.05
        self.module_name = "yarpRTMPose"
        
        self.in_port = yarp.BufferedPortImageRgb()
        self.in_port_name = "/" + self.module_name + "/image:i"
        self.in_port.open(self.in_port_name)

        # TODO: possibly using port and not buffered port is causing delay issues. We want to be able to drop packets
        self.out_image_port = yarp.Port()
        self.out_image_port_name = "/" + self.module_name + "/image:o"
        self.out_image_port.open(self.out_image_port_name)

        self.out_target_port = yarp.Port()
        self.out_target_port_name = "/" + self.module_name + "/target:o"
        self.out_target_port.open(self.out_target_port_name)

        self.in_buf_array = np.ones((image_h,image_w,3), dtype = np.uint8)
        self.in_buf_image = yarp.ImageRgb()
        self.in_buf_image.resize(image_w,image_h)
        self.in_buf_image.setExternal(self.in_buf_array,self.in_buf_array.shape[1],self.in_buf_array.shape[0])

        self.out_buf_array = np.ones((image_h,image_w,3), dtype = np.int8)
        self.out_buf_image = yarp.ImageRgb()
        self.out_buf_image.resize(image_w,image_h)
        self.out_buf_image.setExternal(self.out_buf_array,self.out_buf_array.shape[1],self.out_buf_array.shape[0])

        return True

    def updateModule(self):
        
        received_image = self.in_port.read()
        self.in_buf_image.copy(received_image)

        assert self.in_buf_array.__array_interface__['data'][0] == self.in_buf_image.getRawImage().__int__()

        frame = self.in_buf_array
        skeletons, poses = self.inference(frame)

        target_bottle = yarp.Bottle()
        subtarget_bottle = yarp.Bottle() #Added to preserve compatibility with yarpOpenPose output

        for keypoints in skeletons:

            #keypoints = np.reshape(keypoints,[-1,3])
            skeleton_bottle = yarp.Bottle()

            for n, kp in enumerate(keypoints):
                keypoint_bottle = yarp.Bottle()
                keypoint_bottle.addString(kp)
                keypoint_bottle.addFloat64(keypoints[kp][0].item()) #x
                keypoint_bottle.addFloat32(keypoints[kp][1].item()) #y
                keypoint_bottle.addFloat32(keypoints[kp][2].item()) #score
                skeleton_bottle.addList().read(keypoint_bottle)
            
            subtarget_bottle.addList().read(skeleton_bottle)

        target_bottle.addList().read(subtarget_bottle)

        # Writing output skeletons data
        self.out_target_port.write(target_bottle)

        # Writing output image
        img_skeleton = self.paint(frame,poses,resize=640)
        self.out_buf_array[:,:] = img_skeleton
        self.out_image_port.write(self.out_buf_image)

        return True
    
    def getPeriod(self):
        return self.period

    def cleanup(self):
        self.in_port.close()
        self.out_image_port.close()
        self.out_target_port.close()
        return True

    def interruptModule(self):
        self.in_port.close()
        self.out_image_port.close()
        self.out_target_port.close()
        return True

if __name__ == '__main__':
    yarp.Network.init()

    if not yarp.Network.checkNetwork():
        print("Yarp server is not running")
        quit(-1)

    yarpRTMPose_path = Path(__file__).parent

    rf = yarp.ResourceFinder()
    rf.setVerbose(True)
    rf.setDefaultConfigFile(os.path.join(yarpRTMPose_path,'./app/conf/yarpRTMPose.ini'))
    rf.configure(sys.argv)

    manager = yarpRTMPose()
    manager.runModule(rf)
