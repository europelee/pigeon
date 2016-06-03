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
#include "easylogging++.h"
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
            LOG(ERROR) << "Reconnection failed.";
            reconnect();
        }

        // Re-connection success
        virtual void on_success(const mqtt::itoken& tok) {
            LOG(INFO) << "Reconnection success";
            cli_.get()->subscribe(listener_);
        }

        virtual void connection_lost(const std::string& cause) {
             LOG(INFO) << "Connection lost";
            if (!cause.empty())
                LOG(ERROR) << "cause: " << cause;

            LOG(INFO) << "Reconnecting.";
            nretry_ = 0;
            reconnect();
        }

        virtual void message_arrived(const std::string& topic, mqtt::message_ptr msg) {
            LOG(DEBUG) << "Message arrived";
            LOG(DEBUG) << "topic: '" << topic << "'";
            LOG(DEBUG) << msg->to_str() ;
            if (mPtrMsgListener) {
                mPtrMsgListener.get()->onSuccess(topic, msg->to_str());
            }
        }

        virtual void delivery_complete(mqtt::idelivery_token_ptr token) {

             LOG(DEBUG)<< "Delivery complete for token: " 
                << (token ? token->get_message_id() : -1);
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
