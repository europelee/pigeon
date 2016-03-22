/**
 * @file MqttEnd.h
 * @brief  as a mqttclient, it can pub and sub at the same time.
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-12
 */

#ifndef _MQTTEND_H
#define _MQTTEND_H


#include <string>
#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#include "mqtt/async_client.h"
#include "MqttActionListener.h"

namespace pigeon {

class MqttMsgListener;
class TopicInfo {
    public:
        TopicInfo (const std::string & topicName, int qos):mTopicName(topicName), mQos(qos) {
        };
        std::string mTopicName;
        int         mQos;

    private:

        /** Non-copyable */
        TopicInfo() =delete;
        TopicInfo(TopicInfo &&) =delete;
        TopicInfo& operator=(TopicInfo &&) =delete;
        TopicInfo(const TopicInfo&) =delete;
        TopicInfo& operator=(const TopicInfo&) =delete;
};
class MqttEnd {

    public:
        MqttEnd (const std::string& serverURI, const std::string& clientId, \
                const std::string& userName, const std::string& passWord);

        ~MqttEnd();
        /**
         * @brief subscribe : now just support one topicfilter
         * it is better that change subscribe api params of paho-mqtt client,
         * one vector for topicfilter and one vector for qos---> one map(key: topicfilter, value:qos).
         *
         * @param topicFilter
         * @param li
         */
        void subscribe(const std::string& topicFilter, mqtt::MqttActionListener& li);
        void subscribe(mqtt::MqttActionListener& li);
        void unSubscribe(const std::string & topicFilter);
        int startConnect();
        void setCallBack(mqtt::callback & cb);
        void setTopicFilter(const std::string& topicFilter);
        void setUserName(const std::string& userName);
        void setPassWord(const std::string& passWord);
        int publish(const std::string& topic, const void * msg, size_t len);
        void setMqttMsgListener(std::shared_ptr<MqttMsgListener> ptLi);

    private:
        std::unique_ptr<mqtt::async_client> mAsyncCli; 
        std::string mUserName;
        std::string mPassWord;
        std::vector<std::string>  mTopicFilter; //TopicInfo not used, because of bad design of API subscribe in paho
        std::vector<int>          mQos;
        //std::shared_ptr<MqttMsgListener> mPtrMqttMsgListener;
    private:
        /** Non-copyable */
        MqttEnd() =delete;
        MqttEnd(const MqttEnd&) =delete;
        MqttEnd& operator=(const MqttEnd&) =delete;
        MqttEnd(MqttEnd &&) =delete;
        MqttEnd& operator=(MqttEnd &&) =delete;
};

}

#endif
