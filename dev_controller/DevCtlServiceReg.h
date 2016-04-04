/**
 * @file DevCtlServiceReg.h
 * @brief register service into zookeeper
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-01
 */
#ifndef DEVCTLSERVICE_REG_H
#define DEVCTLSERVICE_REG_H

#include <string>
#include"zookeeper.h"  
#include"zookeeper_log.h"  
#include "zoo_lock.h"

class DevCtlServiceReg {
    public:
        DevCtlServiceReg(const std::string & zkAddrList, const std::string & nodePath, const std::string & nodeValue);
        ~DevCtlServiceReg();
        int RegService();

    private:
        int createNode(zhandle_t *zkhandle, const char * path, int flags);
        int mkdirp(zhandle_t *zkhandle, const char * path, const char * lpath);

    private:
        std::string mZKAddrList;
        std::string mNodePath;
        std::string mNodeValue;
        std::string mNodeRealPath;
        zhandle_t * mZKHdl;

    private:
        /** Non-copyable */
        DevCtlServiceReg(const DevCtlServiceReg&) =delete;
        DevCtlServiceReg& operator=(const DevCtlServiceReg&) =delete;
        DevCtlServiceReg(DevCtlServiceReg &&) =delete;
        DevCtlServiceReg& operator=(DevCtlServiceReg &&) =delete;
};

#endif
