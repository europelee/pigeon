/**
 * @file MattEndTest.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-12
 */
#include <cstdlib>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "MqttActionListener.h"
#include "AsyncMqttCallBack.h"
#include "MqttEnd.h"
#include "MqttMsgListener.h"
#include <pthread.h>
inline void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::shared_ptr<pigeon::MqttEnd> end = nullptr;

class TestListener: public pigeon::MqttMsgListener {

    virtual void onSuccess(const std::string & topic, const std::string & rep) {
        std::cout<<"onSuccess:"<<topic<<"  "<<rep<<std::endl;

        std::string begStr = "/iot/";
        std::string endStr = "/req";
        std::string repEnd = "/rep";
        std::size_t foundBeg = topic.find(begStr);
        if (foundBeg == std::string::npos)
            return;

        std::size_t foundEnd = topic.find(endStr);
        if (foundEnd == std::string::npos)
            return;
        std::string cliID = topic.substr(foundBeg+begStr.size(), foundEnd-foundBeg-begStr.size());
        end.get()->publish(begStr+cliID+repEnd, rep.c_str(), rep.size());
    }

    virtual void onError(int errCode, const std::string & errInfo) {
    }
};
int main(int argc, char ** argv) {
    if (argc < 4) {
        std::cout<<"MqttEndTest clientid username password"<<std::endl;
        return -1;
    }
    mqtt::MqttActionListener li("subListener");
    end = std::make_shared<pigeon::MqttEnd>("tcp://123.57.28.67:1883", argv[1], argv[2], argv[3]);
    auto li2 = std::make_shared<TestListener>();
    mqtt::AsyncMqttCallBack cb(end, li, li2);
    end.get()->setCallBack(cb);
    end.get()->startConnect();
 
    end.get()->subscribe("/iot/+/req", li);
    while (std::tolower(std::cin.get()) != 'q') {
    }
    return 0;
}
