#ifndef EVENT_H
#define EVENT_H

#include <typeindex.h>
#include <map>
#include <vector>
#include <queue>
#include <set>

class event_dispatcher_base
{
public:
    virtual ~event_dispatcher_base(){}
    virtual void post(void*) = 0;
};

class dispatcher_storage
{
public:
    template<typename T>
    static void store(event_dispatcher_base* const disp)
    {
        dispatchers[TypeInfo<T>::Index()].insert(disp);
    }
    template<typename T>
    static void remove(event_dispatcher_base* const disp)
    {
        dispatchers[TypeInfo<T>::Index()].erase(disp);
    }
    template<typename T>
    static void post(const T& e)
    {
        for(auto disp : dispatchers[TypeInfo<T>::Index()])
        {
            disp->post((void*)&e);
        }
    }
private:
    static std::map<typeindex, std::set<event_dispatcher_base*>> dispatchers;
};

template<typename T>
void event_post(const T& e)
{
    dispatcher_storage::post(e);
}

template<typename T>
class event_dispatcher : public event_dispatcher_base
{
public:
    const static unsigned event_limit = 25;
    event_dispatcher()
    {
        dispatcher_storage::store<T>(this);
    }
    ~event_dispatcher()
    {
        dispatcher_storage::remove<T>(this);
    }
    T* poll()
    {
        if(events.empty())
            return 0;
        tmp = events.front();
        events.pop();
        return &tmp;
    }
private:
    void post(void* e)
    {
        if(events.size() < event_limit)
            events.push(*(T*)e);
    }
    T tmp;
    std::queue<T> events;
};

#endif
