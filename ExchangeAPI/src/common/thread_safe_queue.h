#ifndef thread_safe_queue_h
#define thread_safe_queue_h
#include <deque>
#include <boost/thread/mutex.hpp>
template<class T>
class thread_safe_queue
{
public:
    thread_safe_queue(){}
    ~thread_safe_queue(){}
public:
    void push(const T& t)
    {
		boost::mutex::scoped_lock lock(_mutex);
        _queue.push_back(t);
    }
    bool pop(T& t)
    {
        boost::mutex::scoped_lock lock(_mutex);
        if(!_queue.empty())
        {
            t = _queue.front();
            _queue.pop_front();
            return true;
        }
        return false;
    }
    size_t size()
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _queue.size();
    }
private:
    std::deque<T> _queue;
    boost::mutex _mutex;
};



#endif