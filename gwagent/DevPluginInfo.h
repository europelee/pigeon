/**
 * @file DevPluginInfo.h
 * @brief store devplugin info
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-13
 */
#ifndef _DEVPLUGIN_INFO_H
#define _DEVPLUGIN_INFO_H

#include <atomic>
#include <dlfcn.h>
#include "dev_plugin.h"

class DevPluginInfo {
    public:
        DevPluginInfo(void * ptDll, dev_plugin * ptDevPlug):mPtDll(ptDll), mPtDevPlug(ptDevPlug), mActiveFlag(ATOMIC_FLAG_INIT), mBEnd(false) {

        }

        ~DevPluginInfo() {
            mPtDll = NULL;
            free(mPtDevPlug);
            mPtDevPlug = NULL;
            mActiveFlag.clear();

        }

        void * getPlugHandler() const {
            return mPtDll;
        }

        const dev_plugin * getDevPlugin() const {
            return mPtDevPlug;
        }

        bool isEnd() {
            return mBEnd;
        }

        void setEnd(bool flag) {
            mBEnd = flag;
        }

        bool lock() {
            if (!mActiveFlag.test_and_set()) {
                return true;    
            }
            else {
                return false;
            }
        }

        bool unLock() {
            mActiveFlag.clear();
            return true;
        }
    private:
        void * mPtDll;
        dev_plugin * mPtDevPlug;
        std::atomic_flag mActiveFlag;
        volatile bool mBEnd; 

    private:
        /** Non-copyable */
        DevPluginInfo() =delete;
        DevPluginInfo(const DevPluginInfo&) =delete;
        DevPluginInfo& operator=(const DevPluginInfo&) =delete;
        DevPluginInfo(DevPluginInfo &&) =delete;
        DevPluginInfo& operator=(DevPluginInfo &&) =delete;
};
#endif
