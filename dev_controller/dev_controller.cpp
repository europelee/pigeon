/**
 * @file dev_controller.cpp
 * @brief dev_controller for devices control request of user portal
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-26
 */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <atomic>
#include <iostream>
#include <memory>
#include "ZmqEnd.h"
#include "ZmqMsgListener.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "DevQueryEnd.h"
#include "DevOptEnd.h"
#include "DevCtlServiceInfo.h"
#include "DevCtlServiceReg.h"

static std::atomic_flag g_stopflag = ATOMIC_FLAG_INIT;
static std::string  dev_ctl_id = "pigeon";
static std::string  service_path = "/pigeon/devctlsvc/inst-";

std::shared_ptr<pigeon::ZmqEnd> gPtrZmqEndInst = nullptr;
std::unique_ptr<DevQueryEnd> gPtrDqEnd = nullptr;
std::unique_ptr<DevOptEnd> gPtrDOpEnd = nullptr;

static void sigIntHandler(int sig) {
    std::cout<<"sig no "<<sig<<std::endl;
    g_stopflag.clear();
}

class UsrPortalMsgListener : public pigeon::ZmqMsgListener {

    public:
        virtual void onSuccess(const std::string & cliID, const std::string & cliMsg) override {
            std::cout<<"UPMsg:"<<cliID<<" msg:"<<cliMsg<<std::endl;
            /*process UPMsg into MqttMsg*/
            //test
            /**
              rapidjson::Document d;
              d.Parse(cliMsg.c_str());
              rapidjson::Value& seqV = d["seq"];
              rapidjson::Value& devV = d["content"];
              std::cout<<"seq:"<<seqV.GetInt()<<std::endl;//devV.GetString()<<std::endl;
              rapidjson::Value& devtype = d["content"]["devtype"];
              devtype.SetString("query", 5);
              rapidjson::StringBuffer buffer;
              rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
              d.Accept(writer);
              gPtrZmqEndInst->inputQueryRet(cliID, buffer.GetString());
              */
            int seq;
            std::string dest="";
            std::string content = "";

            pigeon::ISCRule::parseU2DevCMsg(cliMsg, seq, dest, content);

            if (dest == DevQueryEnd::DEVQUERYEND_NAME) {
                gPtrDqEnd->procQuery(cliID, seq, content);
                return;
            }

            if (dest == DevOptEnd::DEVOPTEND_NAME) {
                gPtrDOpEnd->procOptCmd(cliID, seq, content);
                return;
            } 
        }

        virtual void onError(int errCode, const std::string & err) override {
            std::cout<<"UPMsg: errcode:"<<errCode<<" errinfo:"<<err<<std::endl;
        }

        ~UsrPortalMsgListener() {
            std::cout<<"UsrPortalMsgListener destructor"<<std::endl;
        }
};


int main(int argc, char ** argv) {

    if (argc < 10) {
        std::cout<<"dev_controller mqtt_clientid mqtt_username mqtt_password mqtt_connuri, zmq_listenuri redisIPAddr redisPort zookeeperAddrList mongodbUri"<<std::endl;
        return -1;
    }
    char hostName[64] = {0};

    int ret = gethostname(hostName, sizeof(hostName)-1);
    if (ret != 0) {
        strcpy(hostName, "pigeon-host");
    }
    
    pid_t procId = getpid();
    char postfix[128] = {0};
    sprintf(postfix, ":%s_%d:", hostName, procId);
    dev_ctl_id += postfix;

    g_stopflag.test_and_set();    
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);

    auto upListener = std::make_shared<UsrPortalMsgListener>();

    gPtrZmqEndInst = std::make_shared<pigeon::ZmqEnd>(argv[5]);
    gPtrZmqEndInst->setMsgListener(upListener);
    gPtrZmqEndInst->startMsgLoop();

    //reg own service into zookeeper
    char svcid[128] = {0};
    sprintf(svcid, "%s_%d", hostName, procId);
    DevCtlServiceInfo sInfo(argv[5], svcid); 
    DevCtlServiceReg  sGeg(argv[8], service_path, sInfo.getServiceInfoJson());
    ret = sGeg.RegService();
    if (ret != 0) {
        
        gPtrZmqEndInst->finMsgLoop();
        return -1;
    }
    //DevQueryEnd
    gPtrDqEnd = std::unique_ptr<DevQueryEnd>(new DevQueryEnd(gPtrZmqEndInst, argv[6], atoi(argv[7])));
    gPtrDqEnd->start();

    //DevOptEnd
    gPtrDOpEnd = std::unique_ptr<DevOptEnd>(new DevOptEnd(dev_ctl_id, gPtrZmqEndInst, argv[1], argv[2], argv[3], argv[4], argv[6], atoi(argv[7]), argv[9]));
    gPtrDOpEnd->start();

    while(g_stopflag.test_and_set()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    gPtrZmqEndInst->finMsgLoop();
    gPtrDqEnd->stop();
    gPtrDOpEnd->stop();
    return 0;
}
