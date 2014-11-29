#ifndef OUTPUT_COLLECTOR_H__
#define OUTPUT_COLLECTOR_H__

// A class to ensure that outputs are written in the proper order in multi-thread
// environments. Modeled after the Moses implementation

#include <boost/thread/mutex.hpp>
#include <pthread.h>
#include <iostream>

namespace travatar {

class OutputCollector {
public:
    OutputCollector(std::ostream* out_stream=&std::cout, std::ostream* err_stream=&std::cerr, bool buffer=true) :
            next_(0), out_stream_(out_stream), err_stream_(err_stream), buffer_(buffer) { }

    void Write(int id, const std::string & out, const std::string & err);
    void Flush();
    void Skip();
private:
    std::map<int,std::pair<std::string,std::string> > saved_;
    int next_;
    std::ostream *out_stream_, *err_stream_;
    boost::mutex mutex_;
    bool buffer_;

};

}

#endif
