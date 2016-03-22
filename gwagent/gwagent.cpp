/**
 * @file gwagent.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-01-25
 */

#include <signal.h>
#include <atomic>
#include <cstdlib>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include "MqttEnd.h"
#include "MsgProcessEnd.h"
#include "AgentFrontEnd.h"
#include "DataCollectEnd.h"

static std::atomic_flag g_stopflag = ATOMIC_FLAG_INIT;
std::shared_ptr<MsgProcessEnd>   MsgProcEnd = nullptr;
std::shared_ptr<pigeon::MqttEnd> end = nullptr;

static void sigIntHandler(int sig) {
    std::cout<<"sig no "<<sig<<std::endl;
    g_stopflag.clear();
}

int main (int argc, char ** argv) {
    if (argc < 6) {
        std::cout<<"gwagent mqtturi clientid username password gwid"<<std::endl;
        return -1;
    }

    g_stopflag.test_and_set();    
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);
    
    MsgProcEnd = std::make_shared<MsgProcessEnd>(argv[5]);

    mqtt::MqttActionListener li("subListener");
    end = std::make_shared<pigeon::MqttEnd>(argv[1], argv[2], argv[3], argv[4]);
    auto li2 = MsgProcEnd;

    std::shared_ptr<AgentFrontEnd>  agentFEnd = std::make_shared<AgentFrontEnd>(li2, end, argv[5]);

    MsgProcEnd.get()->setMPEndObserver(agentFEnd);
    MsgProcEnd->start();
    while(g_stopflag.test_and_set()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    MsgProcEnd->stop();
    return 0;
}

