#ifndef TRAVATAR_TASK_H__
#define TRAVATAR_TASK_H__

namespace travatar {

class Task {
public:
    virtual void Run() = 0;
    virtual ~Task() { }
};

}

#endif
