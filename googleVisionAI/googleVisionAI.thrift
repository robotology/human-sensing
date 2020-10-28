# Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)  
# All Rights Reserved.
# Authors: Vadim Tikhanoff <vadim.tikhanoff@iit.it>
#
# googleVisionAI.thrift

/**
* googleVisionAI_IDL
*
* IDL Interface to google cloud speech processing.
*/

struct Bottle {
} (
  yarp.name = "yarp::os::Bottle"
  yarp.includefile = "yarp/os/Bottle.h"
)

service googleVisionAI_IDL
{
    /**
     * Quit the module.
     * @return true/false on success/failure
     */
    bool quit();

    /**
     * Start annotation.
     * @return true/false on success/failure
     */
    bool annotate();

     /**
     * Get face_annotation.
     * @return a bottle 
     */
    Bottle get_face_annotation();

    /**
     * Get logo_annotation.
     * @return a bottle 
     */
    Bottle get_logo_annotation();

         /**
     * Get text_annotation.
     * @return a bottle 
     */
    Bottle get_text_annotation();

         /**
     * Get safe_search_annotation.
     * @return a bottle 
     */
    Bottle get_safe_search_annotation();
    
}