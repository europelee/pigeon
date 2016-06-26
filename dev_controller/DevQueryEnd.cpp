/**
 * @file DevQueryEnd.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-10
 */
#include <memory>
#include "base.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"
#include "DevQueryEnd.h"
#include "ISCRule.h"
#include "GatewayDataMgr.h"
const std::string & DevQueryEnd::DEVQUERYEND_NAME = "dev_controller";
const std::string & DevQueryEnd::AGENTPROP_ACTION = "getAgentProp";
const std::string & DevQueryEnd::userportal_dest = "userportal";

DevQueryEnd::DevQueryEnd(std::shared_ptr<pigeon::ZmqEnd> zmqEnd, const std::string & redisIp, int port): mSharedPtrZmqEnd(nullptr), mPtrGwDMgr(std::make_shared<pigeon::GatewayDataMgr>(redisIp, port)) {
    mSharedPtrZmqEnd = zmqEnd;
}

DevQueryEnd::~DevQueryEnd() {
    LOG(TRACE)<<"DevQueryEnd destructor";
}

void DevQueryEnd::start() {
    mPtrGwDMgr->startRedisCliProcLoop();
}

void DevQueryEnd::stop() {
    mPtrGwDMgr->stopRedisCliProc();
}

void DevQueryEnd::procQuery(const std::string &cliID, int seq, const std::string &queryJson) {
    /**
     * 1. parse qeryJson and decide call which method of GatewayDataMgr
     * 2. pack cliid, seq, gwid, and callback  to GatewayDataMgr
     * 3. callback would call sharedptrzmqend's method to response msg to up
     */
    rapidjson::Document d;
    d.Parse(queryJson.c_str());
    rapidjson::Value& actionV = d[pigeon::ISCRule::msg_action_field.c_str()];
    rapidjson::Value& gwIdV = d[pigeon::ISCRule::msg_param_field.c_str()][pigeon::ISCRule::msg_gwid_field.c_str()];
    LOG(INFO)<<"action:"<<actionV.GetString()<<" param gwid"<<gwIdV.GetString();
    if (actionV.GetString() == AGENTPROP_ACTION) {
        std::string gwidCp = gwIdV.GetString();
        QFuncObject * ptQf = new QFuncObject(std::bind(&DevQueryEnd::queryCallback, this, cliID, seq, gwidCp, std::placeholders::_1));
        mPtrGwDMgr->getGatewayProp(gwIdV.GetString(), ptQf);
    }
}

void DevQueryEnd::queryCallback(const std::string & cliId, int seq, const std::string & gwid, const std::string & ret) {
    LOG(DEBUG)<<"queryCallback"<<" seq "<<seq<<" gwid"<<gwid;

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    writer.StartObject();
    writer.Key(pigeon::ISCRule::msg_action_field.c_str());
    writer.String(AGENTPROP_ACTION.c_str());
    writer.Key(pigeon::ISCRule::msg_ret_field.c_str());
    writer.StartObject();
    writer.Key(pigeon::ISCRule::msg_gwid_field.c_str());
    writer.String(gwid.c_str());
    writer.Key(pigeon::ISCRule::msg_prop_field.c_str());
    writer.RawValue(ret.c_str(), ret.size(), rapidjson::kObjectType);
    writer.EndObject();
    writer.EndObject();

    std::string msg = pigeon::ISCRule::genDevC2UMsg(seq, userportal_dest, s.GetString());    
    LOG(INFO)<<"retjson:"<<msg;
    mSharedPtrZmqEnd->inputQueryRet(cliId, msg);
} 

