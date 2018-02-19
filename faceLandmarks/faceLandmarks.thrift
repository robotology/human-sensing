# Copyright: (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
# Authors: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
# faceLandmarks.thrift

/**
* faceLandmarks_IDLServer
*
* Interface.
*/

service faceLandmarks_IDLServer
{
  /**
  * Selects what to draw (defaults landmarks on)
  * @param element specifies which element is requested (landmarks | points | labels | dark-mode)
  @param value specifies its value (on | off)
  * @return true/false on success/failure
  */
  bool display(1:string element, 2:string value);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();
}
