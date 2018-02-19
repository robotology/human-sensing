// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_faceLandmarks_IDLServer
#define YARP_THRIFT_GENERATOR_faceLandmarks_IDLServer

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class faceLandmarks_IDLServer;


/**
 * faceLandmarks_IDLServer
 * Interface.
 */
class faceLandmarks_IDLServer : public yarp::os::Wire {
public:
  faceLandmarks_IDLServer();
  /**
   * * Selects what to draw (defaults landmarks on)
   * * @param element specifies which element is requested (landmarks | points | labels | dark-mode)
   * @param value specifies its value (on | off)
   * * @return true/false on success/failure
   */
  virtual bool display(const std::string& element, const std::string& value);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection) YARP_OVERRIDE;
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif
