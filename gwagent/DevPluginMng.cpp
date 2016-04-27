/**
 * @file DevPluginMng.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-14
 */
#include <iostream>
#include <string.h>
#include <utility>
#include <chrono>
#include <thread>
#include "dev_plugin.h"
#include "DevPluginMng.h"

const std::string & DevPluginMng::mDevPlugInitKeyStr = "devplugin_init";
const std::string & DevPluginMng::mDevPlugFinKeyStr = "devplugin_fin";

DevPluginMng::DevPluginMng() {

}

DevPluginMng::~DevPluginMng() {
    mPlugList.clear();
}


bool DevPluginMng::load(const std::string &dllDirPath) {
    bool ret = false;

    this->travelEach(dllDirPath, [this](const std::string & pluginFilePath){

            std::cout<<"loading "<<pluginFilePath<<std::endl;
            if (pluginFilePath.size() <= 0) {
            return;
            }
            
            void * ptDll = dlopen(pluginFilePath.c_str(), RTLD_NOW|RTLD_GLOBAL);
            if (NULL == ptDll) {
            return;
            }

            devplug_init_inf ptFunc = NULL;
            ptFunc = (devplug_init_inf)dlsym(ptDll, DevPluginMng::mDevPlugInitKeyStr.c_str());               
            if (ptFunc == NULL) {
            printf("can not find %s function: %s!\n", DevPluginMng::mDevPlugInitKeyStr.c_str(), dlerror());
            dlclose(ptDll);
            return;
            }
            dev_plugin * ptPlug = (dev_plugin *)malloc(sizeof(dev_plugin));
            ptFunc(ptPlug);
            std::string plugKeyName = ptPlug->dev_pluginname;
            DevPluginInfo * info = new DevPluginInfo(ptDll, ptPlug);
            auto iRet = this->mPlugList.insert(std::make_pair(plugKeyName, info));

            if (iRet.second == true) {
            }
            else {
                std::cout<<plugKeyName<<" already exist!"<<std::endl;
                dlclose(ptDll);
                delete info;
            }
    });

    ret = true;
    return ret;
}

bool DevPluginMng::load(const std::string & dllDirPath, const std::string &plugKeyName) {
    bool ret = false;
    if (NULL != this->getPluginByName(plugKeyName)) {
        std::cout<<plugKeyName<<" already loaded "<<std::endl; 
        ret = false;
    }
    else {
        //start loading
        std::string filePath = "";
        this->travelEach(dllDirPath, [&filePath, &plugKeyName](const std::string & pluginFilePath){

                if (NULL != strcasestr(pluginFilePath.c_str(), plugKeyName.c_str())) {
                filePath = pluginFilePath;
                }

                });

        if (filePath.size() <= 0) {
            ret = false;
            return ret;
        }

        void * ptDll = dlopen(filePath.c_str(), RTLD_NOW|RTLD_GLOBAL);
        if (NULL == ptDll) {
            ret = false;
            return ret;
        }

        devplug_init_inf ptFunc = NULL;
        ptFunc = (devplug_init_inf)dlsym(ptDll, DevPluginMng::mDevPlugInitKeyStr.c_str());               
        if (ptFunc == NULL) {
            printf("can not find %s function: %s!\n", DevPluginMng::mDevPlugInitKeyStr.c_str(), dlerror());
            dlclose(ptDll);
            ret = false;
            return ret;
        }
        dev_plugin * ptPlug = (dev_plugin *)malloc(sizeof(dev_plugin));
        ptFunc(ptPlug);
        DevPluginInfo * info = new DevPluginInfo(ptDll, ptPlug);
        mPlugList.insert(std::make_pair(plugKeyName, info));
    }

    return ret;
}

bool DevPluginMng::unLoad() {
    auto iter = mPlugList.begin();
    for (; iter!=mPlugList.end(); ++iter) {
        DevPluginInfo * pInfo = iter->second;
        if (NULL == pInfo) {
            continue;
        }

        /**
          while(false == pInfo->lock()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
          }
          pInfo->setEnd(true);
          */
        dlclose(pInfo->getPlugHandler());
        //pInfo->unLock();
        delete pInfo;
        pInfo = NULL;
    }
    mPlugList.clear();
    return true;
}

bool DevPluginMng::unLoad(const std::string & plugKeyName) {
    bool ret = false;
    if (NULL == this->getPluginByName(plugKeyName)) {
        std::cout<<plugKeyName<<" not exist!"<<std::endl;
        ret = false;
    }
    else {
        DevPluginInfo * pInfo = mPlugList.find(plugKeyName)->second;

        if (NULL == pInfo) {
            std::cout<<plugKeyName<<" NULL"<<std::endl;
            ret = false;
        }
        else {
            //dont need lock because of multi-thread operation no exist although there exists main-thread and procthread
            /**
              while(false == pInfo->lock()) {
              std::this_thread::sleep_for(std::chrono::milliseconds(20));
              }
              pInfo->setEnd(true);
              */
            dlclose(pInfo->getPlugHandler());
            //pInfo->unLock();
            delete pInfo;
            pInfo = NULL;
            mPlugList.erase(plugKeyName);
            ret = true;
        }
    }

    return ret;    
}


const DevPluginInfo * DevPluginMng::getPluginByName(const std::string & plugKeyName) const {
    auto ptPlug = mPlugList.find(plugKeyName);
    if (ptPlug != mPlugList.end()) {
        return ptPlug->second; 
    }
    else {
        return NULL;
    }
}


const std::map<std::string, DevPluginInfo *> & DevPluginMng::getPlugList() const {
    return mPlugList;
}
