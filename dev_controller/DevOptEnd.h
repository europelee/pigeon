/**
 * @file DevOptEnd.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-18
 */
#ifndef _DEVOPTEND_H
#define _DEVOPTEND_H

#include <memory>
#include <string>
#include "base.h"
#include "ZmqEnd.h"
#include "MqttEnd.h"
#include "MqttActionListener.h"
#include "AsyncMqttCallBack.h"
#include "ISCRule.h"
#include "GatewayDataMgr.h"
#include "QueryInterface.h"
#include "DevDataMgr.h"

class DevOptEnd : public QueryInterface{
    public:
        DevOptEnd(const std::string & devCtlId, std::shared_ptr<pigeon::ZmqEnd> zmqEnd, const std::string & mqtt_clientid, const std::string & mqtt_username, const std::string &mqtt_password, const std::string &mqtt_connuri, const std::string & redisIp, int port, const std::string & mongodbUri);
        ~DevOptEnd();
        void procOptCmd(const std::string & cliID, int seq, const std::string & OptJson);
        void start();
        void stop();    
    private:
        void saveGatewayProp(const std::string & gwId, const std::string & rep);
        bool saveDevData(const std::string & gwid, const std::string & devtype, const std::string & data);
        void delUpRepSub(const std::string & topic); 
        void procGWRepMsg(const std::string & gwId, const std::string & rep);
        virtual void queryCallback(const std::string & cliId, int seq, const std::string & gwid, const std::string & ret) override; 
    public:
        class BrokerMsgListener : public pigeon::MqttMsgListener {
            public:
                DevOptEnd* mPtrDevOptEnd; 

            public:
                BrokerMsgListener():mPtrDevOptEnd(NULL) {
                }
                virtual void onSuccess(const std::string & topic, const std::string & rep) override {
                    LOG(INFO)<<"BRKMsg: topic:"<<topic<<" rep:"<<rep;
                    /*check if msg would be from data collecting or up's request*/

                    /*process MqttMsg into UPMsg rep*/
                    const std::string & gwID0 = pigeon::ISCRule::getUPRepID(topic);
                    if (gwID0 == pigeon::ISCRule::null_id) {
                        LOG(DEBUG) << "not get UPID";
                    }
                    else {
                        /*ZmqEnd back rep*/
                        if (NULL != mPtrDevOptEnd) {
                            mPtrDevOptEnd->delUpRepSub(topic);
                            mPtrDevOptEnd->procGWRepMsg(gwID0, rep); 
                        }
                        return;
                    }

                    //collecting gateway data
                    const std::string & gwID = pigeon::ISCRule::getGWID(topic);
                    if (gwID == pigeon::ISCRule::null_id) {
                        LOG(DEBUG) << "not get GWID";
                    }
                    else {
                        //save into db
                        if (NULL != mPtrDevOptEnd) {
                            mPtrDevOptEnd->saveGatewayProp(gwID, rep);
                        }

                        return;
                    }

                    //collecting devices data
                    std::string gwid = "";
                    std::string devid = "";
                    bool ret = pigeon::ISCRule::getGwDevID(topic, gwid, devid);
                    if (false == ret) {
                        LOG(ERROR)<<"not get GWID, devid FROM "<< topic;
                    }
                    else {
                        
                        LOG(DEBUG)<<"GwID:"<<gwid<<" devID:"<<devid;
                        LOG(DEBUG)<<"devData:"<<rep;
                        
                        if (NULL != mPtrDevOptEnd) {

                            ret = mPtrDevOptEnd->saveDevData(gwid, devid, rep);
                            if (false == ret) {

                                LOG(ERROR)<<"save fail!";
                            }
                        }
                        return;
                    }
                }

                virtual void onError(int errCode, const std::string & errInfo) override {
                    LOG(ERROR)<<"BRKMsg: errCode:"<<errCode<<" errInfo:"<<errInfo;
                }

                ~BrokerMsgListener() {
                    LOG(TRACE)<<"BrokerMsgListener destructor";
                    mPtrDevOptEnd = NULL;
                }
        };

    public:
        static const std::string & DEVOPTEND_NAME;

    private:

        std::shared_ptr<pigeon::ZmqEnd> mSharedPtrZmqEnd;
        std::shared_ptr<pigeon::MqttEnd> gPtrMqttEndInst;

        std::shared_ptr<pigeon::GatewayDataMgr> gPtrGwDMgr;
        std::unique_ptr<pigeon::DevDataMgr>     mUqDevDMgr;
        std::unique_ptr<mqtt::MqttActionListener> gPtrMqttActLi;

        std::shared_ptr<BrokerMsgListener> bkListener;
        mqtt::AsyncMqttCallBack cb;
        uint32_t  cSeq;
        std::string mDevCtlId;

        static const std::string & userportal_dest;
        static const std::string & uprep_sub_tag;
    private:
        /** Non-copyable */
        DevOptEnd(const DevOptEnd&) =delete;
        DevOptEnd& operator=(const DevOptEnd&) =delete;
        DevOptEnd(DevOptEnd &&) =delete;
        DevOptEnd& operator=(DevOptEnd &&) =delete;
};
#endif
