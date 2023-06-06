import sys
import os
import yarp
from yarpRTMPose.inference import yarpRTMPose

# Path to directory of current module
dir_path = os.path.dirname(os.path.realpath(__file__))

# Initialize yarp Network
yarp.Network.init()
if not yarp.Network.checkNetwork():
    print("Yarp server is not running")
    quit(-1)

# Initialize resource finder
rf = yarp.ResourceFinder()
rf.setVerbose(True)
rf.setDefaultConfigFile(os.path.join(dir_path,'..','..','../app/conf/yarpRTMPose.ini'))
rf.configure(sys.argv)

# Start module, this will loop until explicitly interrupted
manager = yarpRTMPose()
manager.runModule(rf)
