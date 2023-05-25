import sys
import os
import yarp
import yarpRTMPose

print("Hi and welcome to yarpRTMPose")
yarp.Network.init()
if not yarp.Network.checkNetwork():
    print("Yarp server is not running")
    quit(-1)

dir_path = os.path.dirname(os.path.realpath(__file__))
print(dir_path)
rf = yarp.ResourceFinder()
rf.setVerbose(True)
rf.setDefaultConfigFile(os.path.join(dir_path,'../app/conf/yarpRTMPose.ini'))
rf.configure(sys.argv)

manager = yarpRTMPose.yarpRTMPose()
manager.runModule(rf)
