#ifndef CONFIG_BASE_H__
#define CONFIG_BASE_H__

#include <string>
#include <vector>
#include <tr1/unordered_map>

namespace travatar {

// name -> value, description
typedef std::tr1::unordered_map<std::string, std::pair<std::string,std::string> > ConfigMap;

class ConfigBase {

protected:

    // argument functions
    int minArgs_, maxArgs_;   // min and max number of main arguments
    std::vector<std::string> mainArgs_; // the main non-optional argiments
    ConfigMap optArgs_;       // optional arguments 

    // details for printing the usage
    std::string usage_;            // usage details
    std::vector<std::string> argOrder_;

public:

    ConfigBase() : minArgs_(0), maxArgs_(255) { }

    void DieOnHelp(const std::string & str) const;
    
    void PrintConf() const;

    std::vector<std::string> LoadConfig(int argc, char** argv, bool print_help = true);
    void LoadConfig(const std::string & file_name);
    void AddConfigEntry(const std::string & name, const std::string & val, const std::string & desc);

    // Getter functions
    std::vector<std::string> GetStringArray(const std::string & name) const;
    const std::string & GetString(const std::string & name) const;
    std::vector<int> GetIntArray(const std::string & name) const;
    int GetInt(const std::string & name) const;
    double GetDouble(const std::string & name) const;
    bool GetBool(const std::string & name) const;

    // Setter functions
    void SetString(const std::string & name, const std::string & val);

    void SetInt(const std::string & name, int val);
    void SetDouble(const std::string & name, double val);
    void SetBool(const std::string & name, bool val);

    void SetUsage(const std::string & str) { usage_ = str; }

    const std::vector<std::string> & GetMainArgs() const { return mainArgs_; }
    const std::string & GetMainArg(int id) const;
	
};

}

#endif
