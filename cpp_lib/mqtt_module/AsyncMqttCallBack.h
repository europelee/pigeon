/**
 * @file AsyncMqttCallBack.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-12
 */
#ifndef _ASYNCMQTTCALLBACK_H
#define _ASYNCMQTTCALLBACK_H

#include <iostream>
#include <string>
#include <memory>
#include "MqttEnd.h"
#include "MqttActionListener.h"
#include <pthread.h>
#include "MqttMsgListener.h"

namespace mqtt {

    class AsyncMqttCallBack : public virtual mqtt::callback,
    public virtual mqtt::iaction_listener

    {
        int nretry_;
        std::shared_ptr<pigeon::MqttEnd> cli_;
        MqttActionListener& listener_;
        std::shared_ptr<pigeon::MqttMsgListener> mPtrMsgListener;
        void reconnect() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            cli_.get()->startConnect();
        }

        // Re-connection failure
        virtual void on_failure(const mqtt::itoken& tok) {
            std::cout << "Reconnection failed." << std::endl;
            reconnect();
        }

        // Re-connection success
        virtual void on_success(const mqtt::itoken& tok) {
            std::cout << "Reconnection success" << std::endl;;
            cli_.get()->subscribe(listener_);
        }

        virtual void connection_lost(const std::string& cause) {
            std::cout << "\nConnection lost" << std::endl;
            if (!cause.empty())
                std::cout << "\tcause: " << cause << std::endl;

            std::cout << "Reconnecting." << std::endl;
            nretry_ = 0;
            reconnect();
        }

        virtual void message_arrived(const std::string& topic, mqtt::message_ptr msg) {
            std::cout << "Message arrived" << std::endl;
            std::cout << "\ttopic: '" << topic << "'" << std::endl;
            std::cout << "\t'" << msg->to_str() << "'\n" << std::endl;
            if (mPtrMsgListener) {
                mPtrMsgListener.get()->onSuccess(topic, msg->to_str());
            }
        }

        virtual void delivery_complete(mqtt::idelivery_token_ptr token) {

            std::cout << "Delivery complete for token: " 
                << (token ? token->get_message_id() : -1) << std::endl;
        }

        private:
        /** Non-copyable */
        AsyncMqttCallBack() =delete;
        AsyncMqttCallBack(const AsyncMqttCallBack&) =delete;
        AsyncMqttCallBack& operator=(const AsyncMqttCallBack&) =delete;
        AsyncMqttCallBack(AsyncMqttCallBack &&) =delete;
        AsyncMqttCallBack& operator=(AsyncMqttCallBack &&) =delete;

        public:
        AsyncMqttCallBack(std::shared_ptr<pigeon::MqttEnd> cli, MqttActionListener& listener,
        std::shared_ptr<pigeon::MqttMsgListener> ptrMsgListener)
            : cli_(cli), listener_(listener), mPtrMsgListener(ptrMsgListener) {}
    };
}
#endif
