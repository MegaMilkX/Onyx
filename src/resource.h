#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>

template<typename T>
class Resource
{
public:
    class Reader
    {
    friend Resource;
    public:
        virtual ~Reader() {}
        virtual void* operator(const std::string& filename) = 0;
    private:
        std:string extension;
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

    static T* Get(const std::string& name)
    {
        T* resource = 0;
        
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
                std::string filename = 
                    searchPaths[i] + 
                    "\\" + 
                    name + 
                    readers[j]->extension;
                resource = readers[j]->(filename);
                if(resource)
                    goto resourceFound;
            }
        }
        
        resourceFound:
        if(resource) referenceCount[name]++;
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
    std::map<std::string, T*> resources;
    std::map<std::string, int> referenceCount;
    std::vector<std::string> searchPaths;
    std::vector<Reader*> readers;
};

#endif
