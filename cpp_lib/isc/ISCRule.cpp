/**
 * @file ISCRule.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-28
 */

#include <sstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "ISCRule.h"

namespace pigeon {

    const std::string & ISCRule::iot_tag = "/iot/";
    const std::string & ISCRule::up_tag = "userportal/";
    const std::string & ISCRule::up_req_tag = "/req";
    const std::string & ISCRule::up_rep_tag = "/rep";
    const std::string & ISCRule::gw_tag = "gateway/";
    const std::string & ISCRule::gw_dev_tag = "/device/";
    const std::string & ISCRule::gw_prop_tag = "/prop";
    const std::string & ISCRule::dev_prop_tag = "/data";
    const std::string & ISCRule::null_id = "";

    const std::string & ISCRule::iot_gw_dbtag = "iot:gateway";
    const std::string & ISCRule::iot_gwprop_dbtag = "prop";

    const std::string & ISCRule::msg_seq_field = "seq";
    const std::string & ISCRule::msg_dest_field = "dest";
    const std::string & ISCRule::msg_content_field = "content";
    const std::string & ISCRule::msg_action_field = "action";
    const std::string & ISCRule::msg_param_field = "param";
    const std::string & ISCRule::msg_ret_field = "ret";
    const std::string & ISCRule::msg_gwid_field = "gwid";
    const std::string & ISCRule::msg_prop_field = "prop";
    const std::string & ISCRule::msg_devices_field = "devices";

    const std::string & ISCRule::devctl_svcinfo_addr_tag = "addr";
    const std::string & ISCRule::devctl_svcinfo_id_tag = "id";

    std::string ISCRule::genDevC2UMsg(int seq, const std::string & dest, const std::string & msgContent) {

        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        writer.Key(msg_seq_field.c_str());
        writer.Int(seq);
        writer.Key(msg_dest_field.c_str());
        writer.String(dest.c_str());
        writer.Key(msg_content_field.c_str());
        //writer.String(msgContent.c_str());
        writer.RawValue(msgContent.c_str(), msgContent.size(), rapidjson::kObjectType);
        writer.EndObject();
        
        return s.GetString();
    }

    bool ISCRule::parseU2DevCMsg(const std::string & msg, int & seq, std::string & dest, std::string & msgContent) {
        
        rapidjson::Document d;
        d.Parse(msg.c_str());
        rapidjson::Value& seqV = d[msg_seq_field.c_str()];
        rapidjson::Value& destV = d[msg_dest_field.c_str()];
        rapidjson::Value& contentV = d[msg_content_field.c_str()];

        seq = seqV.GetInt();
        dest = destV.GetString();

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        contentV.Accept(writer);
        msgContent = sb.GetString();

        return true;
    }

    std::string ISCRule::getGWID(const std::string & topic) {
    
        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos)
            return ISCRule::null_id;

        std::size_t foundEnd = topic.find(gw_prop_tag);
        if (foundEnd == std::string::npos)
            return ISCRule::null_id;

        std::string gwID = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());

        return gwID;
    }
    
    bool ISCRule::getGwDevID(const std::string & topic, std::string & gwid, std::string & devid) {
    
        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos) {
            return false;
        }

        std::size_t foundEnd = topic.find(gw_dev_tag);
        if (foundEnd == std::string::npos) {
            return false;
        }

        gwid = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());

        foundBeg = topic.find(gw_dev_tag);
        if (foundBeg == std::string::npos) {
            return false;
        }

        foundEnd = topic.find(dev_prop_tag);
        if (foundEnd == std::string::npos) {
            return false;
        }

        devid = topic.substr(foundBeg+gw_dev_tag.size(), foundEnd-foundBeg-gw_dev_tag.size());

        return true;
    }

    std::string ISCRule::setGwPropTopic(const std::string & gwid) {
        return std::string(iot_tag+gw_tag+gwid+gw_prop_tag);
    }

    std::string ISCRule::setGwDevDataTopic(const std::string &gwid, const std::string & devid) {
        return std::string(iot_tag+gw_tag+gwid+gw_dev_tag+devid+dev_prop_tag); 
    }

    std::string ISCRule::setUPReqTopic(const std::string & devCId, const std::string & cliID) {

        return std::string(iot_tag+gw_tag+devCId+"/"+cliID+up_req_tag);
    }

    std::string ISCRule::setUPRepTopic(const std::string & devCId, const std::string & cliID) {

        return std::string(iot_tag+gw_tag+devCId+"/"+cliID+up_rep_tag);
    }

    std::string ISCRule::getUPReqID (const std::string & topic) {

        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos)
            return ISCRule::null_id;

        std::size_t foundEnd = topic.find(up_req_tag);
        if (foundEnd == std::string::npos)
            return ISCRule::null_id;

        std::string cliID = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());
        
        std::size_t slashPos = cliID.find("/");
        if (slashPos == std::string::npos) {
            return ISCRule::null_id;
        }
        return cliID.substr(slashPos+1);
    }

    std::string ISCRule::getUPRepID (const std::string & topic) {

        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos)
            return ISCRule::null_id;

        std::size_t foundEnd = topic.find(up_rep_tag);
        if (foundEnd == std::string::npos)
            return ISCRule::null_id;

        std::string cliID = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());

        std::size_t slashPos = cliID.find("/");
        if (slashPos == std::string::npos) {
            return ISCRule::null_id;
        }
        
        return cliID.substr(slashPos+1);
    }

    std::string ISCRule::getUPReqDevCSource (const std::string & topic) {
    
        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos)
            return ISCRule::null_id;

        std::size_t foundEnd = topic.find(up_req_tag);
        if (foundEnd == std::string::npos)
            return ISCRule::null_id;

        std::string cliID = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());
        
        std::size_t slashPos = cliID.find("/");
        if (slashPos == std::string::npos) {
            return ISCRule::null_id;
        }
        return cliID.substr(0, slashPos);
    }

    std::string ISCRule::getUPRepDevCSource (const std::string & topic) {
    
        std::size_t foundBeg = topic.find(iot_tag+gw_tag);
        if (foundBeg == std::string::npos)
            return ISCRule::null_id;

        std::size_t foundEnd = topic.find(up_rep_tag);
        if (foundEnd == std::string::npos)
            return ISCRule::null_id;

        std::string cliID = topic.substr(foundBeg+iot_tag.size()+gw_tag.size(), foundEnd-foundBeg-iot_tag.size()-gw_tag.size());

        std::size_t slashPos = cliID.find("/");
        if (slashPos == std::string::npos) {
            return ISCRule::null_id;
        }
        
        return cliID.substr(0, slashPos);
    }

    std::string ISCRule::genDevC2GWMsg(int seq, const std::string & msgContent) {
            
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        writer.Key(msg_seq_field.c_str());
        writer.Int(seq);
        writer.Key(msg_content_field.c_str());
        writer.RawValue(msgContent.c_str(), msgContent.size(), rapidjson::kObjectType);
        writer.EndObject();
        
        return s.GetString();
    }

    bool ISCRule::parseDevC2GWMsg(const std::string & msg, int & seq, std::string & msgContent) {
    
        rapidjson::Document d;
        d.Parse(msg.c_str());
        rapidjson::Value& contentV = d[pigeon::ISCRule::msg_content_field.c_str()];
        rapidjson::Value& cSeqV = d[pigeon::ISCRule::msg_seq_field.c_str()];
        
        seq = cSeqV.GetInt();

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        contentV.Accept(writer);
        msgContent = sb.GetString();

        return true;
    }

    std::string ISCRule::genGW2DevCMsg(int seq, const std::string & msgContent) {
    
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        writer.StartObject();
        writer.Key(msg_seq_field.c_str());
        writer.Int(seq);
        writer.Key(msg_devices_field.c_str());
        writer.RawValue(msgContent.c_str(), msgContent.size(), rapidjson::kObjectType);
        writer.EndObject();
        
        return s.GetString();
    }

    bool ISCRule::parseGW2DevCMsg(const std::string & msg, int & seq, std::string & msgContent) {
    
        rapidjson::Document d;
        d.Parse(msg.c_str());
        rapidjson::Value& retV = d[pigeon::ISCRule::msg_content_field.c_str()];
        rapidjson::Value& cSeqV = d[pigeon::ISCRule::msg_seq_field.c_str()];
        
        seq = cSeqV.GetInt();

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        retV.Accept(writer);
        msgContent = sb.GetString();

        return true;
    }

}
