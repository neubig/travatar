#include <travatar/config-base.h>
#include <travatar/global-debug.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace std;
using namespace travatar;

#define DIE_HELP(msg) do {                      \
    std::ostringstream oss;                     \
    oss << msg;                                 \
    DieOnHelp(oss.str()); }                     \
  while (0);

void ConfigBase::DieOnHelp(const std::string & str) const {
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

void ConfigBase::PrintConf() const {
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

void ConfigBase::LoadConfig(const string & file_name) {
    ifstream in(file_name.c_str());
    if(!in) THROW_ERROR("Could not open config file " << file_name);
    string line;
    while(getline(in, line)) {
        if(line.size() == 0 || line[0] == '#')
            continue;
        if(line.size() <= 2 || line[0] != '[' || line[line.length()-1] != ']')
            THROW_ERROR("Bad line in config: " << endl << line);
        string id = line.substr(1, line.size()-2);
        ostringstream oss;
        int num = 0;
        while(getline(in, line) && line.size() != 0) {
            if(num++ != 0) oss << ' ';
            oss << line;
        }
        if(num == 0)
            THROW_ERROR("Empty config string for item " << id);
        SetString(id, oss.str());
    }
}

std::vector<std::string> ConfigBase::LoadConfig(int argc, char** argv, bool print_help) {
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

    if(print_help)
        PrintConf();
    return mainArgs_;
}

void ConfigBase::AddConfigEntry(const std::string & name, const std::string & val, const std::string & desc) {
    argOrder_.push_back(name);
    std::pair<std::string,std::pair<std::string,std::string> > entry(name,std::pair<std::string,std::string>(val,desc));
    optArgs_.insert(entry);
}

// Getter functions
std::vector<std::string> ConfigBase::GetStringArray(const std::string & name) const {
    ConfigMap::const_iterator it = optArgs_.find(name);
    if(it == optArgs_.end())
        THROW_ERROR("Requesting bad argument "<<name<<" from configuration");
    std::vector<std::string> ret;
    if(it->second.first.length() != 0)
        boost::algorithm::split(ret, it->second.first, boost::is_any_of("| "));
    return ret;
}
const std::string & ConfigBase::GetString(const std::string & name) const {
    ConfigMap::const_iterator it = optArgs_.find(name);
    if(it == optArgs_.end())
        THROW_ERROR("Requesting bad argument "<<name<<" from configuration");
    return it->second.first;
}
std::vector<int> ConfigBase::GetIntArray(const std::string & name) const {
    std::vector<std::string> arr = GetStringArray(name);
    std::vector<int> ret(arr.size());
    for(int i = 0; i < (int)ret.size(); i++)
        ret[i] = boost::lexical_cast<int>(arr[i]);
    return ret;
}
int ConfigBase::GetInt(const std::string & name) const {
    std::string str = GetString(name);
    int ret = atoi(str.c_str());
    if(ret == 0 && str != "0" && str != "00" && str != "000" && str != "0000")
        DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not an integer");
    return ret;
}
Real ConfigBase::GetReal(const std::string & name) const {
    std::string str = GetString(name);
    Real ret = atof(str.c_str());
    if(ret == 0 && str != "0" && str != "0.0")
        DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not float");
    return ret;
}
bool ConfigBase::GetBool(const std::string & name) const {
    std::string str = GetString(name);
    if(str == "true") return true;
    else if(str == "false") return false;
    DIE_HELP("Value '"<<str<<"' for argument "<<name<<" was not boolean");
    return false;
}

// Setter functions
void ConfigBase::SetString(const std::string & name, const std::string & val) {
    ConfigMap::iterator it = optArgs_.find(name);
    if(it == optArgs_.end())
        THROW_ERROR("Setting bad argument "<<name<<" in configuration");
    it->second.first = val;
}

const std::string & ConfigBase::GetMainArg(int id) const { 
    if(id >= (int)mainArgs_.size())
        throw std::runtime_error("Argument request is out of bounds");
    return mainArgs_[id];
}

void ConfigBase::SetInt(const std::string & name, int val) {
    std::ostringstream oss; oss << val; SetString(name,oss.str());
}
void ConfigBase::SetReal(const std::string & name, Real val) {
    std::ostringstream oss; oss << val; SetString(name,oss.str());
}
void ConfigBase::SetBool(const std::string & name, bool val) {
    std::ostringstream oss; oss << val; SetString(name,oss.str());
}
