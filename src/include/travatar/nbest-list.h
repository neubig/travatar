#ifndef NBEST_LIST_H__
#define NBEST_LIST_H__

#include <vector>
#include <boost/shared_ptr.hpp>

namespace travatar {
// Define a n-best list as a vector of pointers over hyperpaths
class HyperPath;
typedef std::vector< boost::shared_ptr<HyperPath> > NbestList;
}

#endif
