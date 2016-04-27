/**
 * @file MsgProcessEnd.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-17
 */
#include <thread>
#include <list>
#include <functional>
#include "MsgProcessEnd.h"
#include "ISCRule.h"
#include "DataCollectInfo.h"

MsgProcessEnd::MsgProcessEnd(const std::string & gwid): mThreadInter(false), mUqThread(nullptr), mLabelTime(0), mTickTimeLong(25), mPlugShPt(nullptr) {
    std::unique_ptr<DataCollectEnd> tmp(new DataCollectEnd(gwid));
    mUqDCEnd = std::move(tmp);
    int ret = init_memqueue(1024*1024, &mCommCtlInfo, STREAM_IN_DIRECT);
    assert(ret==0);
}


MsgProcessEnd::~MsgProcessEnd() {
    std::cout<<"MsgProcessEnd destructor"<<std::endl;

    int ret = fini_memqueue(1024*1024, &mCommCtlInfo, STREAM_IN_DIRECT); 
    assert(ret==0);
}

mqttMsgInfo * MsgProcessEnd::createMqttMsgInfoObj(const std::string & topic, const std::string & payload) {

    mqttMsgInfo * ptRepO = (mqttMsgInfo *)malloc(sizeof(mqttMsgInfo));
    ptRepO->mPtrTopic = (char *)malloc(topic.size()+1);
    ptRepO->mPtrPayLoad = (char *)malloc(payload.size()+1);
    strcpy(ptRepO->mPtrTopic, topic.c_str());
    strcpy(ptRepO->mPtrPayLoad, payload.c_str());
    return ptRepO;
}

void MsgProcessEnd::releaseMqttMsgInfoObj(mqttMsgInfo ** dPtRepO) {

    if (NULL != dPtRepO && NULL != *dPtRepO) {
        mqttMsgInfo * ptRepO = *dPtRepO;
        free(ptRepO->mPtrTopic);
        ptRepO->mPtrTopic = NULL;
        free(ptRepO->mPtrPayLoad);
        ptRepO->mPtrPayLoad = NULL;
        free(ptRepO);
        ptRepO = NULL;
    }
}

void MsgProcessEnd::start() {
    std::unique_ptr<std::thread> tmp(new std::thread(std::mem_fn(&MsgProcessEnd::procThread), this));
    mUqThread = std::move(tmp);
}

void MsgProcessEnd::stop() {
    mThreadInter = true;
    mUqThread->join();
}

void MsgProcessEnd::procThread() {

    mLabelTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    while(false == mThreadInter) {

        int wRet = mBinSem.timeWait(1);
        if (wRet == -1) {
            if (errno != ETIMEDOUT)
                std::cout<<"procThread:"<<strerror(errno)<<std::endl;
        }
        else { 
            onProc();
        }

        std::time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (nowTime - mLabelTime > mTickTimeLong) {
            onTick();
            mLabelTime = nowTime;
        }

        onCollect();
    }
}    

void MsgProcessEnd::setMPEndObserver(std::shared_ptr<MPEndListener> li) {
    mLi = li;
}

void MsgProcessEnd::setPluginMng(std::shared_ptr<DevPluginMng> mng) {
    mPlugShPt = mng;
}

void MsgProcessEnd::onSuccess(const std::string & topic, const std::string & rep)  {

    std::cout<<"onSuccess:"<<topic<<"  "<<rep<<std::endl;


    /* put cliid&payload into queue*/
    mqttMsgInfo * ptRepO = createMqttMsgInfoObj(topic, rep);
    int ret = shm_write(&mCommCtlInfo, (void *)&ptRepO, sizeof(void *), STREAM_IN_DIRECT);
    if (SHM_OPT_FAIL == ret)
    {
        releaseMqttMsgInfoObj(&ptRepO);
        std::cout<<"shm_write fail"<<std::endl;
        return;
    }
    /* signal BackEndThread*/
    mBinSem.post();

}

void MsgProcessEnd::onError(int errCode, const std::string & errInfo) {

}

void MsgProcessEnd::onProc() {

    /*get msg from queue*/
    mqttMsgInfo * ptRepO = nullptr;
    int ret = shm_read(&mCommCtlInfo, (void *)&ptRepO, sizeof(void *), STREAM_IN_DIRECT);
    if (SHM_OPT_FAIL == ret)
    {
        std::cout<<"shm_read fail"<<std::endl;
        return;
    }

    std::string topic = ptRepO->mPtrTopic;
    std::string rep = ptRepO->mPtrPayLoad;

    releaseMqttMsgInfoObj(&ptRepO);

    const std::string & gwID = pigeon::ISCRule::getUPReqID(topic);
    if (gwID == pigeon::ISCRule::null_id) {
        std::cout<<gwID<<" "<<pigeon::ISCRule::null_id<<std::endl;
        std::cout<<"not get UPID"<<std::endl;
        return;
    }

    const std::string & devCId = pigeon::ISCRule::getUPReqDevCSource(topic);
    if (devCId == pigeon::ISCRule::null_id) {
        std::cout<<devCId<<" "<<pigeon::ISCRule::null_id<<std::endl;
        std::cout<<"not get devCsource"<<std::endl;
        return;
    }
    //todo: parse rep msg then go processing:
    //1. gwagent manager: 
    //(1) reg/devplg-download/self-upgrade;
    //(2) live-checking by collecting agent status)
    //(3) devices management(include collecting device list and their detail infos)
    //2. device control and their properties's collection

    std::shared_ptr<MPEndListener> tmp = mLi.lock();
    if (tmp) {
        const std::string & topic = pigeon::ISCRule::setUPRepTopic(devCId, gwID);
        tmp->onData(topic, rep);
    }
    else {
        std::cout<<"mLi is null"<<std::endl;
    }
}

void MsgProcessEnd::onTick() {

    //
    std::cout<<"collecting ..."<<std::endl;
    const std::list<DataCollectInfo *> & list = mUqDCEnd->startCollection();

    std::shared_ptr<MPEndListener> tmp = mLi.lock();
    if (tmp) {
        std::list<DataCollectInfo *>::const_iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            const DataCollectInfo * info = *it;
            tmp->onData(info->mTopic, info->mPayLoad);
        }
    }
    else {
        std::cout<<"mLi is null"<<std::endl;
    }
}

void MsgProcessEnd::onCollect() {
    if (nullptr == mPlugShPt) {
        return;
    }

    std::time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::map<std::string, DevPluginInfo *> & tmpMap = mPlugShPt->getPlugList();
    for (auto iter = tmpMap.begin(); iter != tmpMap.end(); ++iter) {
        DevPluginInfo * tmp = iter->second;
        if (nowTime - tmp->getLabelTime() > tmp->getDevPlugin()->dev_datacollectTick) {
            //collect
            std::cout<<iter->first<<" collect data ..."<<std::endl;
            char * ptrData = NULL;
            int len = 0;
            tmp->getDevPlugin()->collect_func(&ptrData,&len);
            std::cout<<ptrData<<std::endl;

            const std::list<DataCollectInfo *> & list = mUqDCEnd->startDevDataCollection(iter->first, ptrData);

            std::shared_ptr<MPEndListener> tmp0 = mLi.lock();
            if (tmp0) {
                std::list<DataCollectInfo *>::const_iterator it;
                for (it = list.begin(); it != list.end(); ++it) {
                    const DataCollectInfo * info = *it;
                    tmp0->onData(info->mTopic, info->mPayLoad);
                }
            }
            else {
                std::cout<<"mLi is null"<<std::endl;
            }
            nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            tmp->setLabelTime(nowTime);
        }      
    }   
}
