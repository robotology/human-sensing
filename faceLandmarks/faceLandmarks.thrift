# Copyright: (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
# Authors: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
# faceLandmarks.thrift

/**
* faceLandmarks_IDL
*
* Interface. 
*/

service faceLandmarks_IDL
{
  /**
  * Turns on or off the display of the landmarks
  * @param value (on/off) specifies if diplay is on or off
  * @return true/false on success/failure
  */
  bool display(1:string value);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
