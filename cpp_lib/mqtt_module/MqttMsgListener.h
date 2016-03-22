/**
 * @file MqttMsgListener.h
 * @brief for processing msg arrived from mqtt broker
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-20
 */

#ifndef _MQTTMSGLISTENER_H
#define _MQTTMSGLISTENER_H
namespace pigeon {
    class MqttMsgListener {
        public:
            virtual void onSuccess(const std::string & topic, const std::string & rep)=0;
            virtual void onError(int errCode, const std::string & errInfo)=0;
    };
}
#endif
