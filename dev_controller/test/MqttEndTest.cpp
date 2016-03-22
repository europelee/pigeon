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
class TestListener: public pigeon::MqttMsgListener {

    virtual void onSuccess(const std::string & topic, const std::string & rep) {
        std::cout<<"onSuccess:"<<topic<<"  "<<rep<<std::endl; 
    }

    virtual void onError(int errCode, const std::string & errInfo) {
    }
};
int main(int argc, char ** argv) {
    if (argc < 6) {
        std::cout<<"MqttEndTest clientid username password subTopicPre pubTopicPre"<<std::endl;
        return -1;
    }
    mqtt::MqttActionListener li("subListener");
    std::shared_ptr<pigeon::MqttEnd> end = std::make_shared<pigeon::MqttEnd>("tcp://123.57.28.67:1883", argv[1], argv[2], argv[3]);
    auto li2 = std::make_shared<TestListener>();
    mqtt::AsyncMqttCallBack cb(end, li, li2);
    end.get()->setCallBack(cb);
    end.get()->startConnect();
    int idx = 0;
    char subTopic[128];
    char pubTopic[128];
    char testMsg[128];
    int tok;
    while ((tok = std::tolower(std::cin.get())) != 'q') {

        if (tok == 'a') {
            int cc = idx;
            for (int i = 0; i < 5; ++i) {
                sprintf(subTopic, "%s-%d", argv[4], cc++);
                end.get()->subscribe(subTopic, li);
            }
            cc = idx;
            sleep(4000);

            for (int i = 0; i < 5; ++i) {
                std::cout << "Sending next message..." << std::flush;
                //std::cout<<"tid:"<<pthread_self()<<std::endl;
                sprintf(pubTopic, "%s-%d", argv[5],cc);
                sprintf(testMsg, "%s-%d msg", argv[5], cc++);
                end.get()->publish(pubTopic, testMsg, std::strlen(testMsg));
            }

            idx += 5;
        }
    }

    return 0;
}
