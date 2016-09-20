// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <faceLandmarks_IDL.h>
#include <yarp/os/idl/WireTypes.h>



class faceLandmarks_IDL_display : public yarp::os::Portable {
public:
  std::string value;
  bool _return;
  void init(const std::string& value);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class faceLandmarks_IDL_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool faceLandmarks_IDL_display::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("display",1,1)) return false;
  if (!writer.writeString(value)) return false;
  return true;
}

bool faceLandmarks_IDL_display::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void faceLandmarks_IDL_display::init(const std::string& value) {
  _return = false;
  this->value = value;
}

bool faceLandmarks_IDL_quit::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool faceLandmarks_IDL_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void faceLandmarks_IDL_quit::init() {
  _return = false;
}

faceLandmarks_IDL::faceLandmarks_IDL() {
  yarp().setOwner(*this);
}
bool faceLandmarks_IDL::display(const std::string& value) {
  bool _return = false;
  faceLandmarks_IDL_display helper;
  helper.init(value);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool faceLandmarks_IDL::display(const std::string& value)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool faceLandmarks_IDL::quit() {
  bool _return = false;
  faceLandmarks_IDL_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool faceLandmarks_IDL::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool faceLandmarks_IDL::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
    if (tag == "display") {
      std::string value;
      if (!reader.readString(value)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = display(value);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "quit") {
      bool _return;
      _return = quit();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "help") {
      std::string functionName;
      if (!reader.readString(functionName)) {
        functionName = "--all";
      }
      std::vector<std::string> _return=help(functionName);
      yarp::os::idl::WireWriter writer(reader);
        if (!writer.isNull()) {
          if (!writer.writeListHeader(2)) return false;
          if (!writer.writeTag("many",1, 0)) return false;
          if (!writer.writeListBegin(BOTTLE_TAG_INT, static_cast<uint32_t>(_return.size()))) return false;
          std::vector<std::string> ::iterator _iterHelp;
          for (_iterHelp = _return.begin(); _iterHelp != _return.end(); ++_iterHelp)
          {
            if (!writer.writeString(*_iterHelp)) return false;
           }
          if (!writer.writeListEnd()) return false;
        }
      reader.accept();
      return true;
    }
    if (reader.noMore()) { reader.fail(); return false; }
    yarp::os::ConstString next_tag = reader.readTag();
    if (next_tag=="") break;
    tag = tag + "_" + next_tag;
  }
  return false;
}

std::vector<std::string> faceLandmarks_IDL::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("display");
    helpString.push_back("quit");
    helpString.push_back("help");
  }
  else {
    if (functionName=="display") {
      helpString.push_back("bool display(const std::string& value) ");
      helpString.push_back("Turns on or off the display of the landmarks ");
      helpString.push_back("@param value (on/off) specifies if diplay is on or off ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="quit") {
      helpString.push_back("bool quit() ");
      helpString.push_back("Quit the module. ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="help") {
      helpString.push_back("std::vector<std::string> help(const std::string& functionName=\"--all\")");
      helpString.push_back("Return list of available commands, or help message for a specific function");
      helpString.push_back("@param functionName name of command for which to get a detailed description. If none or '--all' is provided, print list of available commands");
      helpString.push_back("@return list of strings (one string per line)");
    }
  }
  if ( helpString.empty()) helpString.push_back("Command not found");
  return helpString;
}


