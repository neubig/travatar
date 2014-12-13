#include<travatar/output-collector.h>

using namespace travatar;
using namespace std;
using namespace boost;

void OutputCollector::Write(int id, const string & out, const string & err) { 
    typedef map<int,pair<string,string> > TraceMap;
    boost::mutex::scoped_lock lock(mutex_);
    if(id == next_) {
        *out_stream_ << out;
        *err_stream_ << err;
        TraceMap::iterator it;
        for(++next_; (it = saved_.find(next_)) != saved_.end(); ++next_) {
            *out_stream_ << it->second.first;
            *err_stream_ << it->second.second;
            saved_.erase(it);
        }
        if(!buffer_) {
            out_stream_->flush();
            err_stream_->flush();
        }
    } else {
        saved_[id] = pair<string,string>(out,err);
    }
}

void OutputCollector::Flush() {
    out_stream_->flush();
    err_stream_->flush();
}

void OutputCollector::Skip() {
    ++next_;
}
