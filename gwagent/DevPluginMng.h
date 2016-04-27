
#ifndef DEVPLUGIN_MNG_H
#define DEVPLUGIN_MNG_H

#include <string>
#include <map>
#include <functional>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DevPluginInfo.h"
class DevPluginMng {
    public:
        DevPluginMng();
        ~DevPluginMng();
    
        bool load(const std::string & dllDirPath);
        bool unLoad();
        bool load(const std::string & dllDirPath, const std::string & plugKeyName);
        bool unLoad(const std::string & plugKeyName);
        
        const DevPluginInfo * getPluginByName(const std::string & plugKeyName) const;
        const std::map<std::string, DevPluginInfo *> & getPlugList() const;
    private:
        
        template<class Function>
        void travelEach(const std::string & dllDirPath, Function fn) {
        
            //if more than one file matched with plugkeyName, just select one simplily
            DIR *d;
            struct dirent *file;
            struct stat sb;
            char filename[256] ={0};

            if(!(d = opendir(dllDirPath.c_str())))
            {
                printf("error opendir %s!!!\n", dllDirPath.c_str());
                return ;
            }

            while((file = readdir(d)) != NULL)
            {

                if(strncmp(file->d_name, ".", 1) == 0 || strncmp(file->d_name, "..", 2) == 0)
                    continue;

                if(stat(file->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode))
                {
                    //just level one travel
                    continue;
                }


                sprintf(filename, "%s/%s", dllDirPath.c_str(), file->d_name);
                fn(filename);
            }

            closedir(d);
        }
    private:
        std::map<std::string, DevPluginInfo *> mPlugList;
        static const std::string & mDevPlugInitKeyStr;
        static const std::string & mDevPlugFinKeyStr;
    private:
        /** Non-copyable */
        DevPluginMng(const DevPluginMng&) =delete;
        DevPluginMng& operator=(const DevPluginMng&) =delete;
        DevPluginMng(DevPluginMng &&) =delete;
        DevPluginMng& operator=(DevPluginMng &&) =delete;
};
#endif
