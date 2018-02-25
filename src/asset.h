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
        Resource<T>::AddSearchPath(path);
    }
    template<typename R>
    static void add_reader(const std::string& ext)
    {
        Resource<T>::AddReader<R>(ext);
    }
    
    void set(const std::string& name)
    {
        *this = Resource<T>::Get(name);
    }
    void operator=(const T* other)
    {
        this->data = (T*)other;
    }
    T* get() { return data; }
private:
    T* data;
};

#endif
