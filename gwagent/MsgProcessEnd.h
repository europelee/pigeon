/**
 * @file MsgProcessEnd.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-17
 */
#ifndef _MSGPROCESS_END_H
#define _MSGPROCESS_END_H
#include <string>
#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include "DataCollectEnd.h"
#include "binarysemaphore.h"
#include "MqttMsgListener.h"
#include "MPEndListener.h"

#ifdef __cplusplus
    extern "C" {
#endif

#include "shm_comm.h"

#ifdef __cplusplus        
    }
#endif


typedef struct mqttMsgInfo {
    char * mPtrTopic;
    char * mPtrPayLoad;
}mqttMsgInfo;

class MsgProcessEnd : public pigeon::MqttMsgListener {
    public:
        MsgProcessEnd(const std::string & gwid);
        ~MsgProcessEnd();
        void setMPEndObserver(std::shared_ptr<MPEndListener> li);
        void start();
        void stop();
    public:

        virtual void onSuccess(const std::string & topic, const std::string & rep) override;

        virtual void onError(int errCode, const std::string & errInfo) override;

    private:
        std::weak_ptr<MPEndListener>  mLi;
        std::atomic<bool>   mThreadInter;
        pigeon::BinarySemaphore  mBinSem;            
        chn_comm_ctlinfo  mCommCtlInfo;
        std::unique_ptr<std::thread>  mUqThread;
        std::unique_ptr<DataCollectEnd> mUqDCEnd;
        std::time_t  mLabelTime;    
        const int   mTickTimeLong;
    private:

        mqttMsgInfo * createMqttMsgInfoObj(const std::string & topic, const std::string & payload);
        void    releaseMqttMsgInfoObj(mqttMsgInfo ** dPtRepO);
        void procThread();
        void onProc();
        void onTick();

    private:
        /** Non-copyable */
        MsgProcessEnd(const MsgProcessEnd&) =delete;
        MsgProcessEnd& operator=(const MsgProcessEnd&) =delete;
        MsgProcessEnd(MsgProcessEnd &&) =delete;
        MsgProcessEnd& operator=(MsgProcessEnd &&) =delete;
};
#endif
