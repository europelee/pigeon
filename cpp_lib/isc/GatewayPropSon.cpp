/**
 * @file GatewayPropSon.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-04
 */
/**
 * todo: check valid with json-schema from developers's commit
 *
 */
#include <iostream>
#include "easylogging++.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"
#include "GatewayPropSon.h"

namespace pigeon {
    
    const char * GatewayPropSon::online_proptag = "online";

    GatewayPropSon::GatewayPropSon():online(-1) {
    }
    
    GatewayPropSon::~GatewayPropSon() {
    }
    
    const char * GatewayPropSon::getOnLineTag() {
        return online_proptag;
    }
    
    std::string GatewayPropSon::getPropJson() {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);

        writer.StartObject();
        writer.String(online_proptag);
        writer.Int(online);
        writer.EndObject();
        
        return std::string(s.GetString());
    } 
    
    bool GatewayPropSon::parsePropJson(const char * propJson) {
        
        bool bRet = false;
        rapidjson::Document d;
        if (false == d.Parse(propJson)) {
            LOG(DEBUG)<<"Parse inputJson fail";
            return false;
        }
        if (d[online_proptag].IsInt()) {
            online = d[online_proptag].GetInt();
            bRet = true;
        }
        else {
            LOG(ERROR)<<online_proptag<<" value not int";
            bRet = false; 
        }

        return bRet;
    }

    void GatewayPropSon::setOnLine(bool bOn) {
        online = (bOn==true ? 1 : 0);
    }

    int GatewayPropSon::getOnLine() {
        return online;
    }
}
