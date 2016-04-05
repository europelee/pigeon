/**
 * @file AgentFrontEnd.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-17
 */
#ifndef _AGENT_FRONTEND_H
#define _AGENT_FRONTEND_H

#include <memory>
#include "MqttEnd.h"
#include "AsyncMqttCallBack.h"
#include "MqttMsgListener.h"
#include "MqttActionListener.h"
#include "MPEndListener.h"
#include "ISCRule.h"
class AgentFrontEnd : public MPEndListener {
    public:
        AgentFrontEnd(std::shared_ptr<pigeon::MqttMsgListener> li, std::shared_ptr<pigeon::MqttEnd> end, const std::string & gwid):mShrdPtrMqttEnd(end), mLi("subListener"), cb(mShrdPtrMqttEnd, mLi, li){
            mShrdPtrMqttEnd.get()->setCallBack(cb);
            mShrdPtrMqttEnd.get()->startConnect();
            mShrdPtrMqttEnd.get()->subscribe(pigeon::ISCRule::setUPReqTopic("+", gwid), mLi);            
        }

        ~AgentFrontEnd() {
            std::cout<<"AgentFrontEnd destructor"<<std::endl;
        }
    public:
        virtual void onData(const std::string & topic, const std::string & content) override {
            mShrdPtrMqttEnd.get()->publish(topic, content.c_str(), content.size());
        }

    private:
        std::shared_ptr<pigeon::MqttEnd> mShrdPtrMqttEnd;
        mqtt::MqttActionListener mLi;
        mqtt::AsyncMqttCallBack cb;

    private:
        /** Non-copyable */
        AgentFrontEnd() =delete;
        AgentFrontEnd(const AgentFrontEnd&) =delete;
        AgentFrontEnd& operator=(const AgentFrontEnd&) =delete;
        AgentFrontEnd(AgentFrontEnd &&) =delete;
        AgentFrontEnd& operator=(AgentFrontEnd &&) =delete;
};

#endif
