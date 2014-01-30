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
    OutputCollector(std::ostream* out_stream=&std::cout, std::ostream* err_stream=&std::cerr) :
            next_(0), out_stream_(out_stream), err_stream_(err_stream) { }

    void Write(int id, const std::string & out, const std::string & err) {
        boost::mutex::scoped_lock lock(mutex_);
        if(id == next_) {
            *out_stream_ << out;
            *err_stream_ << err;
            std::map<int,std::pair<std::string,std::string> >::iterator it;
            for(++next_; (it = saved_.find(next_)) != saved_.end(); ++next_) {
                *out_stream_ << it->second.first;
                *err_stream_ << it->second.second;
                saved_.erase(it);
            }
            out_stream_->flush();
            err_stream_->flush();
        } else {
            saved_[id] = std::pair<std::string,std::string>(out,err);
        }
    }

private:
    std::map<int,std::pair<std::string,std::string> > saved_;
    int next_;
    std::ostream *out_stream_, *err_stream_;
    boost::mutex mutex_;

};

}

#endif
