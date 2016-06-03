/**
 * @file MqttActionListener.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-12
 */
#ifndef _ACTION_LISTENER_H
#define _ACTION_LISTENER_H

#include <iostream>
#include <string>
#include "easylogging++.h"
#include "mqtt/async_client.h"

namespace mqtt {
    class MqttActionListener : public virtual mqtt::iaction_listener
    {
        std::string name_;

        virtual void on_failure(const mqtt::itoken& tok) {
            LOG(ERROR) << name_ << " failure";
            if (tok.get_message_id() != 0)
                LOG(ERROR) << " (token: " << tok.get_message_id() << ")" ;

        }

        virtual void on_success(const mqtt::itoken& tok) {
            LOG(TRACE) << name_ << " success";
            if (tok.get_message_id() != 0)
                LOG(DEBUG) << " (token: " << tok.get_message_id() << ")";
            if (!tok.get_topics().empty())
                LOG(DEBUG) << "\ttoken topic: '" << tok.get_topics()[0] << "', ...";

        }


        private:
        /** Non-copyable */
        MqttActionListener() =delete;
        MqttActionListener(const MqttActionListener&) =delete;
        MqttActionListener& operator=(const MqttActionListener&) =delete;
        MqttActionListener(MqttActionListener &&) =delete;
        MqttActionListener& operator=(MqttActionListener &&) =delete;

        public:
        MqttActionListener(const std::string& name) : name_(name) {}
    };
}
#endif
