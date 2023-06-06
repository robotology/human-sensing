import numpy as np
import yarp
import sys
from importlib.util import spec_from_file_location, module_from_spec 
from collections import defaultdict
from mmdeploy_runtime import Detector, PoseDetector
import cv2

import yarpRTMPose.format_mapping as fmt

class RTMPose():

    def __init__(self,det_model,pose_model,dataset,device):
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
            
        self.keypoint_info = dataset_module.dataset_info["keypoint_info"]
        self.keypoint_op_info = {} 
        for i,keypoints in enumerate(self.keypoint_info):
            self.keypoint_op_info[i] = {"name": fmt.COCO_WHOLEBODY_TO_OP.get(self.keypoint_info[i]["name"],
                                                                self.keypoint_info[i]["name"])}

    def inference(self,img):
        bboxes, labels, _ = self.detector(img)
        keep = np.logical_and(labels == 0, bboxes[...,4] > 0.6)
        bboxes = bboxes[keep, :4]
        poses = self.pose_detector(img,bboxes)

        return poses

    def format_keypoints(self,poses,openpose_format=False):

        if openpose_format:
            keypoint_info = self.keypoint_op_info
        else:
            keypoint_info = self.keypoint_info

        keypoints = []
        for person in poses:
            person_keypoints = {} 
            for i, keypoint in enumerate(person):
                person_keypoints[keypoint_info[i]["name"]] = keypoint 

            keypoints.append(person_keypoints)
        
        if openpose_format:
            for kp in fmt.OP_TO_COCO_WHOLEBODY:
                for person in keypoints:
                    if kp not in person:
                        person[kp] = np.array([0.0, 0.0, 0.0], dtype=np.float32) #Add fake keypoint to keep compatibility w/ op

            openpose_keypoints = [] 
            for person in keypoints: #kp is a dict {'kp_name': [x,y,score]}
                openpose_person_keypoints = defaultdict(list)
                for kp in person:
                    if 'face' not in kp:
                        openpose_person_keypoints[kp] = person[kp]
                    else:
                        openpose_person_keypoints['face'].append(person[kp])
                openpose_keypoints.append(openpose_person_keypoints)

            return openpose_keypoints

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
        yarp.RFModule.__init__(self)

    def configure(self,rf):

        yarp.Network.init()
        
        self.image_h = rf.find("height").asInt16()
        self.image_w = rf.find("width").asInt16()

        self.period = 0.01
        self.module_name = 'yarpRTMPose'

        self.det_model = rf.find("det_model_path").asString()
        self.pose_model = rf.find("pose_model_path").asString()
        self.dataset = rf.find("dataset").asString()
        self.device = rf.find("device").asString() 
        self.openpose_format = rf.find("openpose_format").asBool() 
        RTMPose.__init__(self,self.det_model,self.pose_model,self.dataset,self.device)

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

        self.in_buf_array = np.ones((self.image_h,self.image_w,3), dtype = np.uint8)
        self.in_buf_image = yarp.ImageRgb()
        self.in_buf_image.resize(self.image_w,self.image_h)
        self.in_buf_image.setExternal(self.in_buf_array,self.in_buf_array.shape[1],self.in_buf_array.shape[0])

        self.out_buf_array = np.ones((self.image_h,self.image_w,3), dtype = np.int8)
        self.out_buf_image = yarp.ImageRgb()
        self.out_buf_image.resize(self.image_w,self.image_h)
        self.out_buf_image.setExternal(self.out_buf_array,self.out_buf_array.shape[1],self.out_buf_array.shape[0])

        return True

    def updateModule(self):
        
        received_image = self.in_port.read()
        self.in_buf_image.copy(received_image)

        assert self.in_buf_array.__array_interface__['data'][0] == self.in_buf_image.getRawImage().__int__()

        frame = self.in_buf_array
        poses = self.inference(frame)

        skeletons = self.format_keypoints(poses,self.openpose_format)

        target_bottle = yarp.Bottle()
        subtarget_bottle = yarp.Bottle() #Added to preserve compatibility with yarpOpenPose output

        for keypoints in skeletons:

            #keypoints = np.reshape(keypoints,[-1,3])
            skeleton_bottle = yarp.Bottle()
            for n, kp in enumerate(keypoints):
                if self.openpose_format and kp == 'face':
                    face_bottle = yarp.Bottle()
                    face_bottle.addString('face')
                    for face_kp in keypoints[kp]:
                        face_kp_bottle = yarp.Bottle()
                        face_kp_bottle.addFloat32(face_kp[0].item())
                        face_kp_bottle.addFloat32(face_kp[1].item())
                        face_kp_bottle.addFloat32(face_kp[2].item())
                        face_bottle.addList().read(face_kp_bottle)
                    
                    skeleton_bottle.addList().read(face_bottle)
                else:
                    keypoint_bottle = yarp.Bottle()
                    keypoint_bottle = yarp.Bottle()
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
        img_skeleton = self.paint(frame,poses,resize=max(self.image_h,self.image_w))
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
