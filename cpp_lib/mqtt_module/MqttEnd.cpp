/**
 * @file MqttEnd.cpp
 * @brief as mqttclient, it can pub and sub.
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-12
 */
#include <iostream>
#include <algorithm>
#include "MqttEnd.h"
#include "MqttActionListener.h"
#include "MqttMsgListener.h"
namespace pigeon {

    MqttEnd::MqttEnd(const std::string& serverURI, const std::string& clientId, \
             const std::string& userName, const std::string& passWord)
        :mAsyncCli(new mqtt::async_client(serverURI, clientId)), mUserName(userName), \
       mPassWord(passWord) {
    
    }

    MqttEnd::~MqttEnd() {
        std::cout << "MqttEnd destructor" << std::endl;
		std::cout << "Disconnecting..." << std::endl;
		mqtt::itoken_ptr conntok = mAsyncCli->disconnect();
		conntok->wait_for_completion();
    }


    void MqttEnd::setUserName(const std::string &userName) {
        mUserName = userName;        
    }

    void MqttEnd::setPassWord(const std::string& passWord) {
        mPassWord = passWord;
    }

    void MqttEnd::subscribe(const std::string& topicFilter, mqtt::MqttActionListener& li) {
		//not check if the same topic name exist
        mTopicFilter.push_back(topicFilter);
        mQos.push_back(1);
        mAsyncCli->subscribe(topicFilter, 1, nullptr, li);
    }

    void MqttEnd::subscribe(mqtt::MqttActionListener& li) {
		mAsyncCli->subscribe(mTopicFilter, mQos, nullptr, li);
    }

    void MqttEnd::setCallBack(mqtt::callback & cb) {
        mAsyncCli->set_callback(cb);
    }
    
    void MqttEnd::unSubscribe(const std::string & topicFilter) {
        auto iter = std::find(mTopicFilter.begin(), mTopicFilter.end(), topicFilter);
        if (iter != mTopicFilter.end()) {
            mTopicFilter.erase(iter);
            mQos.pop_back();
        }
        mAsyncCli->unsubscribe(topicFilter);
    }

    void MqttEnd::setTopicFilter(const std::string& topicFilter) {
        mTopicFilter.push_back(topicFilter);
        mQos.push_back(1);
    }

    int MqttEnd::startConnect() {
        int iRet = 0; 
        mqtt::connect_options connOpts;
        connOpts.set_keep_alive_interval(20);
        connOpts.set_clean_session(true);
        connOpts.set_user_name(mUserName);
        connOpts.set_password(mPassWord);
        try {
            mqtt::itoken_ptr conntok = mAsyncCli->connect(connOpts);
            std::cout << "Waiting for the connection..." << std::flush;
            conntok->wait_for_completion();
            std::cout << "OK" << std::endl;

        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
            iRet = -1;
            return iRet;
        }

        return iRet;

    }

    int MqttEnd::publish(const std::string& topic, const void* msg, size_t len) {

        mAsyncCli->publish(topic, msg, len, 1,false);
        return 0;
    }
}
