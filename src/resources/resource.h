#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <vector>

template<typename T>
class resource
{
public:
    class reader
    {
    friend resource;
    public:
        virtual ~reader() {}
        virtual T* operator()(const std::string& filename) = 0;
    private:
        std::string extension;
    };

    static void add_search_path(const std::string& path)
    { searchPaths.push_back(path); }
    
    template<typename READER>
    static void add_reader(const std::string& extension)
    {
        READER* rdr = new READER();
        rdr->extension = extension;
        readers.push_back(rdr);
    }
    
    static void Set(const std::string& name, T*);

    static bool exists(const std::string& name)
    {
        return (bool)resources.count(name);
    }

    static T* get(const std::string& name)
    {
        T* res = 0;
        std::string filename;
        std::map<std::string, T*>::iterator it =
            resources.begin();
        for(it; it != resources.end(); ++it)
        {
            if(it->first == name)
            {
                res = it->second;
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
                
                res = (T*)readers[j]->operator()(filename);
                
                if(res)
                {
                    resources[name] = res;
                    goto resourceFound;
                }
            }
        }
        
        resourceFound:
        if(res) referenceCount[name]++;
        else std::cout << "resource not found: " << filename << std::endl;
        return res;
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
    static std::vector<reader*> readers;
};

template<typename T>
std::map<std::string, T*> resource<T>::resources;
template<typename T>
std::map<std::string, int> resource<T>::referenceCount;
template<typename T>
std::vector<std::string> resource<T>::searchPaths;
template<typename T>
std::vector<typename resource<T>::reader*> resource<T>::readers;

#endif
