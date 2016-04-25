/**
 * @file DevCtlServiceInfo.h
 * @brief the info would be saved into zookeeper
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-01
 */

#ifndef DEVCTLSERVICEINFO_H
#define DEVCTLSERVICEINFO_H

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"
#include "ISCRule.h"
class DevCtlServiceInfo {
    public:
        DevCtlServiceInfo(const std::string & svcAddr, const std::string svcId): mSvcAddr(svcAddr), mSvcId(svcId), mInfoJson(""){
        }

        ~DevCtlServiceInfo() {
        }

        const std::string & getAddr() const {
            return mSvcAddr;
        }

        const std::string & getId() const {
            return mSvcId;
        }

        std::string getServiceInfoJson() {
            
            if (mInfoJson != "") {
                return mInfoJson;
            }

            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            writer.StartObject();
            writer.Key(pigeon::ISCRule::devctl_svcinfo_addr_tag.c_str());
            writer.String(mSvcAddr.c_str());
            writer.Key(pigeon::ISCRule::devctl_svcinfo_id_tag.c_str());
            writer.String(mSvcId.c_str());
            writer.EndObject();
            
            mInfoJson = s.GetString();

            return mInfoJson;

        }

    private:
        std::string mSvcAddr;
        std::string mSvcId;
        std::string mInfoJson;

    private:
        /** Non-copyable */
        DevCtlServiceInfo(const DevCtlServiceInfo&) =delete;
        DevCtlServiceInfo& operator=(const DevCtlServiceInfo&) =delete;
        DevCtlServiceInfo(DevCtlServiceInfo &&) =delete;
        DevCtlServiceInfo& operator=(DevCtlServiceInfo &&) =delete;
};

#endif
