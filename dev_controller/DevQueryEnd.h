/**
 * @file DevQueryEnd.h
 * @brief accept user portal request and sending dev props
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-09
 */
#ifndef _DEVQUERYEND_H
#define _DEVQUERYEND_H

#include <memory>
#include "ZmqEnd.h"
#include "GatewayDataMgr.h"
#include "QueryInterface.h"

class DevQueryEnd : public QueryInterface {

    public:
        DevQueryEnd(std::shared_ptr<pigeon::ZmqEnd> zmqEnd, const std::string & redisIp, int port); 
        ~DevQueryEnd(); 
        void start();
        void stop();
        void procQuery(const std::string &cliID, int seq, const std::string &queryJson); 

    public:
        static const std::string & DEVQUERYEND_NAME;

    public:
        virtual void queryCallback(const std::string & cliId, int seq, const std::string & gwid, const std::string & ret) override; 

    private:
        static const std::string & AGENTPROP_ACTION;
        static const std::string & userportal_dest;

    private:
        std::shared_ptr<pigeon::ZmqEnd> mSharedPtrZmqEnd;
        std::shared_ptr<pigeon::GatewayDataMgr> mPtrGwDMgr;
    private:
        /** Non-copyable */
        DevQueryEnd(const DevQueryEnd&) =delete;
        DevQueryEnd& operator=(const DevQueryEnd&) =delete;
        DevQueryEnd(DevQueryEnd &&) =delete;
        DevQueryEnd& operator=(DevQueryEnd &&) =delete;
};
#endif
