#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <vector>

template<typename T>
class Resource
{
public:
    class Reader
    {
    friend Resource;
    public:
        virtual ~Reader() {}
        virtual T* operator()(const std::string& filename) = 0;
    private:
        std::string extension;
    };

    static void AddSearchPath(const std::string& path)
    { searchPaths.push_back(path); }
    
    template<typename READER>
    static void AddReader(const std::string& extension)
    {
        READER* reader = new READER();
        reader->extension = extension;
        readers.push_back(reader);
    }
    
    static void Set(const std::string& name, T*);

    static T* Get(const std::string& name)
    {
        T* resource = 0;
        std::string filename;
        std::map<std::string, T*>::iterator it =
            resources.begin();
        for(it; it != resources.end(); ++it)
        {
            if(it->first == name)
            {
                resource = it->second;
                goto resourceFound;
            }
        }
        
        
        for(unsigned i = 0; i < searchPaths.size(); ++i)
        {
            for(unsigned j = 0; j < readers.size(); ++j)
            {
                // TODO: REMOVE PLATFORM DEPENDENT CODE
                // TODO: RESPECT EMPTY SEARCH PATHS
                // TODO: MAKE FULL FILE PATH
                filename = 
                    searchPaths[i] + 
                    "\\" + 
                    name +
                    "." +
                    readers[j]->extension;
                
                resource = (T*)readers[j]->operator()(filename);
                
                if(resource)
                {
                    resources[name] = resource;
                    goto resourceFound;
                }
            }
        }
        
        resourceFound:
        if(resource) referenceCount[name]++;
        else std::cout << "Resource not found: " << filename << std::endl;
        return resource;
    }
    
    static void Free(const std::string& name)
    {
        referenceCount[name]--;
        if(referenceCount[name] > 0)
            return;
        
        std::map<std::string, T*>::iterator it =
            resources.begin();
        for(it; it != resources.end(); ++it)
        {
            if(it->first == name)
            {
                delete it->second;
                resources.erase(name);
            }
        }
    }
    
private:
    static std::map<std::string, T*> resources;
    static std::map<std::string, int> referenceCount;
    static std::vector<std::string> searchPaths;
    static std::vector<Reader*> readers;
};

template<typename T>
std::map<std::string, T*> Resource<T>::resources;
template<typename T>
std::map<std::string, int> Resource<T>::referenceCount;
template<typename T>
std::vector<std::string> Resource<T>::searchPaths;
template<typename T>
std::vector<typename Resource<T>::Reader*> Resource<T>::readers;

#endif
