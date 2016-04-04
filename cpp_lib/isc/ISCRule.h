/**
 * @file ISCRule.h
 * @brief for routing msg between sevices
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-27
 */
#ifndef _ISCRULE_H
#define _ISCRULE_H
#include <iostream>
#include <string>
namespace pigeon {
    class ISCRule {
        public:
            static const std::string & iot_tag;
            static const std::string & up_tag;
            static const std::string & up_req_tag;
            static const std::string & up_rep_tag;
            static const std::string & gw_tag;
            static const std::string & gw_prop_tag;
            static const std::string & null_id;
            
            static const std::string & iot_gw_dbtag;
            static const std::string & iot_gwprop_dbtag;

            static const std::string & msg_seq_field;
            static const std::string & msg_dest_field;
            static const std::string & msg_content_field;
            static const std::string & msg_action_field;
            static const std::string & msg_param_field;
            static const std::string & msg_ret_field;
            static const std::string & msg_gwid_field;
            static const std::string & msg_prop_field;
            static const std::string & msg_devices_field;

            static const std::string & devctl_svcinfo_addr_tag;
        public:
            static std::string genDevC2UMsg(int seq, const std::string & dest, const std::string & msgContent);
            static bool parseU2DevCMsg(const std::string & msg, int & seq, std::string & dest, std::string & msgContent);
            static std::string genDevC2GWMsg(int seq, const std::string & msgContent);
            static bool parseDevC2GWMsg(const std::string & msg, int & seq, std::string & msgContent);
            static std::string genGW2DevCMsg(int seq, const std::string & msgContent);
            static bool parseGW2DevCMsg(const std::string & msg, int & seq, std::string & msgContent); 
            static std::string getGWID(const std::string & topic);
            static std::string setGwPropTopic(const std::string & gwid);
            static std::string setUPReqTopic(const std::string & cliID);
            static std::string setUPRepTopic(const std::string & cliID);
            static std::string getUPReqID (const std::string & topic);
            static std::string getUPRepID (const std::string & topic);
    };
}
#endif
