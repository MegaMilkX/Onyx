#ifndef ASSET_H
#define ASSET_H

#include "resources/resource.h"

template<typename T>
class asset
{
public:
    asset()
    : data(0) 
    {}
    static void add_search_path(const std::string& path)
    {
        resource<T>::add_search_path(path);
    }
    template<typename R>
    static void add_reader(const std::string& ext)
    {
        resource<T>::add_reader<R>(ext);
    }
    
    void set(const std::string& name)
    {
        *this = resource<T>::get(name);
    }
    void operator=(const T* other)
    {
        this->data = (T*)other;
    }

    T* operator->() const { return data; }
    T* get() { return data; }
private:
    T* data;
};

#endif
