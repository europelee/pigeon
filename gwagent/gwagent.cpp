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
#include "easylogging++.h"
#include "MqttEnd.h"
#include "MsgProcessEnd.h"
#include "AgentFrontEnd.h"
#include "DataCollectEnd.h"
#include "DevPluginMng.h"

INITIALIZE_EASYLOGGINGPP

static std::atomic_flag g_stopflag = ATOMIC_FLAG_INIT;
static const std::string plugDirPath = "./dev_plugins";

std::shared_ptr<MsgProcessEnd>   MsgProcEnd = nullptr;
std::shared_ptr<pigeon::MqttEnd> end = nullptr;
std::shared_ptr<DevPluginMng> plugMng = nullptr;

static void sigIntHandler(int sig) {
    LOG(TRACE)<<"sig no "<<sig;
    g_stopflag.clear();
}

int main (int argc, char ** argv) {
    if (argc < 6) {
        std::cout<<"gwagent mqtturi clientid username password gwid"<<std::endl;
        return -1;
    }

    el::Configurations confFromFile("./gwagent-logger.conf");
    el::Loggers::reconfigureAllLoggers(confFromFile);
    
    g_stopflag.test_and_set();    
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);
   
    plugMng = std::make_shared<DevPluginMng>();
    plugMng->load(plugDirPath);

    MsgProcEnd = std::make_shared<MsgProcessEnd>(argv[5]);

    mqtt::MqttActionListener li("subListener");
    end = std::make_shared<pigeon::MqttEnd>(argv[1], argv[2], argv[3], argv[4]);
    auto li2 = MsgProcEnd;

    std::shared_ptr<AgentFrontEnd>  agentFEnd = std::make_shared<AgentFrontEnd>(li2, end, argv[5]);

    MsgProcEnd.get()->setMPEndObserver(agentFEnd);
    MsgProcEnd->setPluginMng(plugMng);

    MsgProcEnd->start();
    while(g_stopflag.test_and_set()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    MsgProcEnd->stop();
    plugMng->unLoad();
    return 0;
}

