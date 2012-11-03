#ifndef CONFIG_BASE_H__
#define CONFIG_BASE_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <travatar/util.h>
#include <boost/algorithm/string.hpp>

namespace travatar {

#define DIE_HELP(msg) do {                      \
    std::ostringstream oss;                     \
    oss << msg;                                 \
    DieOnHelp(oss.str()); }                     \
  while (0);

class ConfigBase {

protected:

    // name -> value, description
    typedef StringMap<std::pair<std::string,std::string> > ConfigMap;

    // argument functions
    int minArgs_, maxArgs_;   // min and max number of main arguments
    std::vector<std::string> mainArgs_; // the main non-optional argiments
    ConfigMap optArgs_;       // optional arguments 

    // details for printing the usage
    std::string usage_;            // usage details
    std::vector<std::string> argOrder_;

public:

    ConfigBase() : minArgs_(0), maxArgs_(255) { }

    void DieOnHelp(const std::string & str) const {
        // print arguments
        std::cerr << usage_ << std::endl;
        std::cerr << "Arguments: "<<std::endl;
        for(std::vector<std::string>::const_iterator it = argOrder_.begin(); 
                        it != argOrder_.end(); it++) {
            ConfigMap::const_iterator oit = optArgs_.find(*it);
            if(oit->second.second.length() != 0)
                std::cerr << " -"<<oit->first<<" \t"<<oit->second.second<<std::endl;
        }
        std::cerr << std::endl << str << std::endl;
        exit(1);
    }
    
    void PrintConf() const {
        // print arguments
        std::cerr << "Main arguments:" << std::endl;
        for(int i = 0; i < (int)mainArgs_.size(); i++)
            std::cerr << " "<<i<<": "<<mainArgs_[i]<<std::endl;
        std::cerr << "Optional arguments:"<<std::endl;
        for(std::vector<std::string>::const_iterator it = argOrder_.begin(); it != argOrder_.end(); it++) {
            ConfigMap::const_iterator oit = optArgs_.find(*it);
            if(oit->second.second.length() != 0)
                std::cerr << " -"<<oit->first<<" \t"<<oit->second.first<<std::endl;
        }
    }

    std::vector<std::string> loadConfig(int argc, char** argv) {
        for(int i = 1; i < argc; i++) {
            if(argv[i][0] == '-') {
                std::string name(argv[i]+1); 
                ConfigMap::iterator cit = optArgs_.find(name);
                if(cit == optArgs_.end())
                    DIE_HELP("Illegal argument "<<name);
                if(i == argc-1 || argv[i+1][0] == '-')
                    cit->second.first = "true";
                else
                    cit->second.first = argv[++i];
            }
            else
                mainArgs_.push_back(argv[i]);
        }

        // sanity checks
        if((int)mainArgs_.size() < minArgs_ || (int)mainArgs_.size() > maxArgs_) {
            DIE_HELP("Wrong number of arguments");
        }

        PrintConf();
        return mainArgs_;
    }

    void AddConfigEntry(const std::string & name, const std::string & val, const std::string & desc) {
        argOrder_.push_back(name);
        std::pair<std::string,std::pair<std::string,std::string> > entry(name,std::pair<std::string,std::string>(val,desc));
        optArgs_.insert(entry);
    }

    // Getter functions
    std::vector<std::string> GetStringArray(const std::string & name) const {
        ConfigMap::const_iterator it = optArgs_.find(name);
        if(it == optArgs_.end())
            THROW_ERROR("Requesting bad argument "<<name<<" from configuration");
        std::vector<std::string> ret;
        if(it->second.first.length() != 0)
            boost::algorithm::split(ret, it->second.first, boost::is_any_of("|"));
        return ret;
    }
    const std::string & GetString(const std::string & name) const {
        ConfigMap::const_iterator it = optArgs_.find(name);
        if(it == optArgs_.end())
            THROW_ERROR("Requesting bad argument "<<name<<" from configuration");
        return it->second.first;
    }
    int GetInt(const std::string & name) const {
        std::string str = GetString(name);
        int ret = atoi(str.c_str());
        if(ret == 0 && str != "0" && str != "00" && str != "000" && str != "0000")
            DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not an integer");
        return ret;
    }
    double GetDouble(const std::string & name) const {
        std::string str = GetString(name);
        double ret = atof(str.c_str());
        if(ret == 0 && str != "0" && str != "0.0")
            DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not float");
        return ret;
    }
    bool GetBool(const std::string & name) const {
        std::string str = GetString(name);
        if(str == "true") return true;
        else if(str == "false") return false;
        DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not boolean");
        return false;
    }

    // Setter functions
    void SetString(const std::string & name, const std::string & val) {
        ConfigMap::iterator it = optArgs_.find(name);
        if(it == optArgs_.end())
            THROW_ERROR("Setting bad argument "<<name<<" in configuration");
        it->second.first = val;
    }
    void SetInt(const std::string & name, int val) {
        std::ostringstream oss; oss << val; SetString(name,oss.str());
    }
    void SetDouble(const std::string & name, double val) {
        std::ostringstream oss; oss << val; SetString(name,oss.str());
    }
    void SetBool(const std::string & name, bool val) {
        std::ostringstream oss; oss << val; SetString(name,oss.str());
    }

    void SetUsage(const std::string & str) { usage_ = str; }

    const std::vector<std::string> & GetMainArgs() const { return mainArgs_; }
	
};

}

#endif
