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
#include "easylogging++.h"
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
    LOG(TRACE)<<"MsgProcessEnd destructor";
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
    mDataValidator.init();
    std::unique_ptr<std::thread> tmp(new std::thread(std::mem_fn(&MsgProcessEnd::procThread), this));
    mUqThread = std::move(tmp);
}

void MsgProcessEnd::stop() {
    mDataValidator.fin();
    mThreadInter = true;
    mUqThread->join();
}

void MsgProcessEnd::procThread() {

    mLabelTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    while(false == mThreadInter) {

        int wRet = mBinSem.timeWait(1);
        if (wRet == -1) {
            if (errno != ETIMEDOUT)
                LOG(ERROR)<<"procThread:"<<strerror(errno);
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

    LOG(DEBUG)<<"onSuccess:"<<topic<<"  "<<rep;


    /* put cliid&payload into queue*/
    mqttMsgInfo * ptRepO = createMqttMsgInfoObj(topic, rep);
    int ret = shm_write(&mCommCtlInfo, (void *)&ptRepO, sizeof(void *), STREAM_IN_DIRECT);
    if (SHM_OPT_FAIL == ret)
    {
        releaseMqttMsgInfoObj(&ptRepO);
        LOG(ERROR)<<"shm_write fail";
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
        LOG(ERROR)<<"shm_read fail";
        return;
    }

    std::string topic = ptRepO->mPtrTopic;
    std::string rep = ptRepO->mPtrPayLoad;

    releaseMqttMsgInfoObj(&ptRepO);

    const std::string & gwID = pigeon::ISCRule::getUPReqID(topic);
    if (gwID == pigeon::ISCRule::null_id) {
        LOG(TRACE)<<gwID<<" "<<pigeon::ISCRule::null_id<<" not get upid";
        return;
    }

    const std::string & devCId = pigeon::ISCRule::getUPReqDevCSource(topic);
    if (devCId == pigeon::ISCRule::null_id) {
        LOG(TRACE)<<devCId<<" "<<pigeon::ISCRule::null_id<<" not get devSource";
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
        LOG(ERROR)<<"mLi is null";
    }
}

void MsgProcessEnd::onTick() {

    //
    LOG(TRACE)<<"collecting ...";
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
        LOG(ERROR)<<"mLi is null";
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
            LOG(DEBUG)<<iter->first<<" collect data ...";
            char * ptrData = NULL;
            int len = 0;
            tmp->getDevPlugin()->collect_func(&ptrData,&len);
            LOG(DEBUG)<<ptrData;
            if (false == mDataValidator.validJsonData(ptrData)) {
                free(ptrData);
                ptrData = NULL;
                nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                tmp->setLabelTime(nowTime);
                continue;
            }
            const std::list<DataCollectInfo *> & list = mUqDCEnd->startDevDataCollection(iter->first, ptrData);
            free(ptrData);
            ptrData = NULL;
            std::shared_ptr<MPEndListener> tmp0 = mLi.lock();
            if (tmp0) {
                std::list<DataCollectInfo *>::const_iterator it;
                for (it = list.begin(); it != list.end(); ++it) {
                    const DataCollectInfo * info = *it;
                    tmp0->onData(info->mTopic, info->mPayLoad);
                }
            }
            else {
                LOG(ERROR)<<"mLi is null";
            }
            nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            tmp->setLabelTime(nowTime);
        }      
    }   
}
