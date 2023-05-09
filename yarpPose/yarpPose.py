import numpy as np
import yarp
import os
import sys
from pathlib import Path
from importlib import import_module

class yarpPose(yarp.RFModule):

    def __init__(self,name):
        self.module_name = name
        self.period = 0.1
        super().__init__()
        
    def inference(self,img):
        pass

    def configure(self,rf):

        image_h = rf.find("height").asInt16()
        image_w = rf.find("width").asInt16()

        yarp.Network.init()

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
        skeletons = self.inference(frame)

        target_bottle = yarp.Bottle()
        subtarget_bottle = yarp.Bottle() #Added to preserve compatibility with yarpOpenPose output

        for keypoints in skeletons:

            #keypoints = np.reshape(keypoints,[-1,3])
            skeleton_bottle = yarp.Bottle()

            for n, kp in enumerate(keypoints):
                keypoint_bottle = yarp.Bottle()
                keypoint_bottle.addString(kp)
                keypoint_bottle.addFloat32(keypoints[kp][0]) #x
                keypoint_bottle.addFloat32(keypoints[kp][1]) #y
                keypoint_bottle.addFloat32(keypoints[kp][2]) #score
                skeleton_bottle.addList().read(keypoint_bottle)
            
            subtarget_bottle.addList().read(skeleton_bottle)

        target_bottle.addList().read(subtarget_bottle)

        # Writing output skeletons data
        self.out_target_port.write(target_bottle)

        # Writing output image
        self.out_buf_array[:,:] = frame
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

    def build(self,rf):
        '''
            Factory method that builds the subclass specified in the configuration file
        '''
        try:
            module = import_module(self.module_name)
        except:
            print("There is no such module available, using yarpMMPose by default")
            self.module_name = 'yarpMMPose'
            module = import_module(self.module_name)
            
        cls = getattr(module, self.module_name) #The class must have the same name of the module
        inferencer = cls.fromconfig(rf)
        
        return inferencer


if __name__ == '__main__':
    yarp.Network.init()

    if not yarp.Network.checkNetwork():
        print("Yarp server is not running")
        quit(-1)

    yarpPose_path = Path(__file__).parent

    rf = yarp.ResourceFinder()
    rf.setVerbose(True)
    rf.setDefaultConfigFile(os.path.join(yarpPose_path,'app/conf/yarpPose.ini'))
    rf.configure(sys.argv)

    module_name = rf.find("module").asString()
    manager = yarpPose(module_name).build(rf)
    #manager = yarpMMPose()
    manager.runModule(rf)