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
#include "mqtt/async_client.h"

namespace mqtt {
    class MqttActionListener : public virtual mqtt::iaction_listener
    {
        std::string name_;

        virtual void on_failure(const mqtt::itoken& tok) {
            std::cout << name_ << " failure";
            if (tok.get_message_id() != 0)
                std::cout << " (token: " << tok.get_message_id() << ")" << std::endl;
            std::cout << std::endl;
        }

        virtual void on_success(const mqtt::itoken& tok) {
            std::cout << name_ << " success";
            if (tok.get_message_id() != 0)
                std::cout << " (token: " << tok.get_message_id() << ")" << std::endl;
            if (!tok.get_topics().empty())
                std::cout << "\ttoken topic: '" << tok.get_topics()[0] << "', ..." << std::endl;
            std::cout << std::endl;
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
