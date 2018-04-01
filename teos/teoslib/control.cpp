#include <stdio.h>
#include <iostream>
#include <cstdarg>

#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

#ifdef WIN32
#else
#include <pthread.h>
#endif

#include <teoslib/config.h>
#include <teoslib/control.hpp>

std::string formatUsage(std::string unixUsage) {
#ifdef WIN32
  std::string windowsCmndUsage = unixUsage;
  boost::replace_all(windowsCmndUsage, "\"", "\"\"\"");
  boost::replace_all(windowsCmndUsage, "'", "\"");
  return windowsCmndUsage;
#else
  return unixUsage;
#endif
}

namespace teos
{
  using namespace std;
  using namespace boost::property_tree;

#define SHARP "#  "
  boost::format output(string label, string format) {
    string header = (boost::format(SHARP "%1$20s: ") % label).str();
    return boost::format(header + format);
  }

  void output(const char* label, const char* format, ...) {
    printf(SHARP "%20s: ", label);

    string f(format);
    f += "\n";

    va_list argptr;
    va_start(argptr, format);
    vprintf(f.c_str(), argptr);
    va_end(argptr);
  }

  void output(const char* text, ...) {
    printf(SHARP "%s\n", text);
  }

  ostream& sharp() {
    cout << SHARP;
    return cout;
  }

  void output(string text){
    text = SHARP + text;
    boost::replace_all<string>(text, "\n", "\n" SHARP);
    cout << text << endl;
  }  

  string TeosControl::requestToString(bool isRaw) const {
    stringstream ss;
    json_parser::
      write_json(ss, reqJson_, !isRaw);
    return ss.str();
  }

  string TeosControl::responseToString(bool isRaw) const {
    stringstream ss;
    json_parser::
      write_json(ss, respJson_, !isRaw);
    return ss.str();
  }

  string TeosControl::getConfigJson(){
    string configJson 
      = (boost::filesystem::path(boost::filesystem::current_path()) 
      / CONFIG_JSON).string();
    return configJson;
  }

  ptree TeosControl::getConfig(bool verbose) {
    ptree config;
    try
    {
      read_json(getConfigJson(), config);
    }
    catch (...) {
      if (verbose) {
        printf("ERROR: Cannot read the config file: %s!\n"
          , getConfigJson().c_str());
      }
    }
    return config;
  }

  ptree TeosControl::errorRespJson(string sender, string message) {
    ptree respJson;

    if(!sender.empty()) {
      string senderEntry = "\"sender\":\"" + sender + "\"";
      string msgEntry = "\"message\":{" + message + "}";
      respJson.put(teos_ERROR, "{" + senderEntry + ", " + msgEntry + "}");
    } else {
      respJson.put(teos_ERROR, message);
    }
    isError_ = true;  
    return respJson; 
  }

  void TeosControl::putError(string message, string sender) {
    respJson_ = errorRespJson(sender, message);
  }
}
