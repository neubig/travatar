#include <travatar/thread-pool.h>
#include <travatar/global-debug.h>
#include <travatar/task.h>

using namespace travatar;
using namespace std;
using namespace boost;

ThreadPool::ThreadPool(int num_threads, int queue_limit) :
        stopped_(false), stopping_(false), delete_tasks_(true),
        queue_limit_(queue_limit) {
    for(int i = 0; i < num_threads; i++)
        threads_.create_thread(bind(&ThreadPool::Run, this));
}

void ThreadPool::Run() {
    while(!stopped_) {
        Task* task = NULL;
        {
            mutex::scoped_lock lock(mutex_);
            if(tasks_.empty() && !stopped_)
                thread_needed_.wait(lock);
            if(!stopped_ && !tasks_.empty()) {
                task = tasks_.front();
                tasks_.pop();
            }
        }
        if(task) {
            task->Run();
            if(delete_tasks_)
                delete task;
        }
        thread_available_.notify_all();
    }
}

void ThreadPool::Submit(Task* task) {
    mutex::scoped_lock lock(mutex_);
    if(stopping_)
        THROW_ERROR("Cannot accept new jobs while ThreadPool is stopping");
    while(queue_limit_ && (int)tasks_.size() >= queue_limit_)
        thread_available_.wait(lock);
    tasks_.push(task);
    thread_needed_.notify_all();
}

void ThreadPool::Wait() {
    while(!tasks_.empty()) {
        boost::mutex::scoped_lock lock(mutex_);
        thread_available_.wait(lock);
    }
    sleep(1);
}

void ThreadPool::Stop(bool process_remaining) {
    {
        mutex::scoped_lock lock(mutex_);
        if(stopped_) return;
        stopping_ = true;
    }
    if(process_remaining) {
        boost::mutex::scoped_lock lock(mutex_);
        while(!tasks_.empty() && !stopped_)
            thread_available_.wait(lock);
    }
    {
        boost::mutex::scoped_lock lock(mutex_);
        stopped_ = true;
    }
    thread_needed_.notify_all();
    threads_.join_all();
}
