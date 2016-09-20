// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_faceLandmarks_IDL
#define YARP_THRIFT_GENERATOR_faceLandmarks_IDL

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class faceLandmarks_IDL;


/**
 * faceLandmarks_IDL
 * Interface.
 */
class faceLandmarks_IDL : public yarp::os::Wire {
public:
  faceLandmarks_IDL();
  /**
   * Turns on or off the display of the landmarks
   * @param value (on/off) specifies if diplay is on or off
   * @return true/false on success/failure
   */
  virtual bool display(const std::string& value);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif
