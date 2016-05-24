/**
 * @file DevOptEnd.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-18
 */
#include <sstream>
#include "DevOptEnd.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

const std::string & DevOptEnd::DEVOPTEND_NAME = "gwagent";
const std::string & DevOptEnd::userportal_dest = "userportal";
const std::string & DevOptEnd::uprep_sub_tag = "subscribe";

DevOptEnd::DevOptEnd(const std::string & devCtlId, std::shared_ptr<pigeon::ZmqEnd> zmqEnd, const std::string & mqtt_clientid, const std::string & mqtt_username, const std::string &mqtt_password, const std::string &mqtt_connuri, const std::string & redisIp, int port, const std::string & mongodbUri):
    mSharedPtrZmqEnd(zmqEnd), gPtrMqttEndInst(std::make_shared<pigeon::MqttEnd>(mqtt_connuri, mqtt_clientid, mqtt_username, mqtt_password)),
    gPtrGwDMgr(std::make_shared<pigeon::GatewayDataMgr>(redisIp, port)),
    mUqDevDMgr(new pigeon::DevDataMgr(mongodbUri, pigeon::ISCRule::iot_devdata_db_id, pigeon::ISCRule::iot_devcollection_id)),    
    gPtrMqttActLi(new mqtt::MqttActionListener("subListener")), 
    bkListener(std::make_shared<BrokerMsgListener>()), 
    cb(gPtrMqttEndInst, *gPtrMqttActLi, bkListener), cSeq(0), mDevCtlId(devCtlId) {

        gPtrMqttEndInst.get()->setCallBack(cb);
        gPtrMqttEndInst.get()->startConnect();
        gPtrMqttEndInst.get()->subscribe("/iot/gateway/+/prop", *gPtrMqttActLi);
        gPtrMqttEndInst->subscribe("/iot/gateway/+/device/+/data", *gPtrMqttActLi);
    }

DevOptEnd::~DevOptEnd() {
    std::cout<<"DevOptEnd destructor"<<std::endl;
}

void DevOptEnd::start() {
    bkListener->mPtrDevOptEnd = this;
    gPtrGwDMgr->startRedisCliProcLoop();

}

void DevOptEnd::stop() {
    gPtrGwDMgr->stopRedisCliProc(); 
}


void DevOptEnd::queryCallback(const std::string & cliId, int seq, const std::string & gwid, const std::string & ret) {

    if (cliId == pigeon::ISCRule::null_id) {
        std::cout<<"queryCallback "+ret+" null"<<std::endl;
        return;
    }

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    writer.StartObject();
    writer.Key(pigeon::ISCRule::msg_gwid_field.c_str());
    writer.String(gwid.c_str());
    writer.Key(pigeon::ISCRule::msg_ret_field.c_str());
    writer.RawValue(ret.c_str(), ret.size(), rapidjson::kObjectType);
    writer.EndObject();
    
    std::string msg = pigeon::ISCRule::genDevC2UMsg(seq, userportal_dest, s.GetString());    
    std::cout<<"retjson:"<<msg<<std::endl;
    mSharedPtrZmqEnd->inputBackEndMsg(cliId, msg);
} 

void DevOptEnd::procOptCmd(const std::string & cliID, int seq, const std::string & optJson) {
    
    std::stringstream ss;
    ss<<cSeq;
    std::string optId = mDevCtlId + ss.str();

    rapidjson::Document d;
    d.Parse(optJson.c_str());
    rapidjson::Value& gwIdV = d[pigeon::ISCRule::msg_gwid_field.c_str()];
    rapidjson::Value& devCmdV = d[pigeon::ISCRule::msg_devices_field.c_str()];

    gPtrGwDMgr->cacheUpRouterInfo(optId, cliID, seq, gwIdV.GetString());
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    devCmdV.Accept(writer);
    
    std::string cliMsg = pigeon::ISCRule::genDevC2GWMsg(cSeq, sb.GetString());

    /*MqttEnd sub & publish*/
    std::string repTopic = pigeon::ISCRule::setUPRepTopic(optId, gwIdV.GetString());    
    gPtrGwDMgr->cacheUpRepSub(mDevCtlId+uprep_sub_tag, repTopic);
    gPtrMqttEndInst.get()->subscribe(repTopic, *gPtrMqttActLi);

    gPtrMqttEndInst.get()->publish(pigeon::ISCRule::setUPReqTopic(optId, gwIdV.GetString()), cliMsg.c_str(), cliMsg.size());
    ++cSeq;
}

void DevOptEnd::saveGatewayProp(const std::string & gwId, const std::string & rep) {
    gPtrGwDMgr->saveGatewayProp(gwId, rep);
}

bool DevOptEnd::saveDevData(const std::string & gwid, const std::string & devtype, const std::string & data) {
    return mUqDevDMgr->saveDevData(gwid, devtype, data);
}

void DevOptEnd::delUpRepSub(const std::string & topic) {
    gPtrGwDMgr->delUpRepSub(mDevCtlId+uprep_sub_tag, topic);
}

void DevOptEnd::procGWRepMsg(const std::string & gwId, const std::string & rep) {

    //proc msg into (cliID, rep), send to up
   /**
    * 1. get cSeq from rep, then get <cliid, seq, gwid??redundance> by quering redis by cSeq. 
    * 2. pack data to up(cliid, <seq, dest, rep.ret>)
    */
    int cSeq = -1;
    std::string ret = "";
    pigeon::ISCRule::parseGW2DevCMsg(rep, cSeq, ret);
    
    std::stringstream ss;
    ss<<cSeq;
    std::string optId = mDevCtlId + ss.str();
    
    QCacheFuncObject * ptQf = new QCacheFuncObject(std::bind(&DevOptEnd::queryCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ret));
    gPtrGwDMgr->getUpRouterInfo(optId, ptQf);
}
