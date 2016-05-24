/**
 * @file DevDataMgr.h
 * @brief processing data storing and quering related with device data
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-05-23
 */
#ifndef _DEVDATAMGR_H
#define _DEVDATAMGR_H

#include <string>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/pipeline.hpp>
namespace pigeon { 
    class DevDataMgr {

        public:
            DevDataMgr(const std::string & dbUri, const std::string & devDBName, const std::string & devColName);
            ~DevDataMgr();
            bool saveDevData(const std::string & gwid, const std::string & devtype, const std::string & data);

        private:
            mongocxx::instance      mMongoCxxInst;
            mongocxx::client        mMongoCli;
            mongocxx::collection    mDevDataCollection;

        private:
            /** Non-copyable */
            DevDataMgr(const DevDataMgr&) =delete;
            DevDataMgr& operator=(const DevDataMgr&) =delete;
            DevDataMgr(DevDataMgr &&) =delete;
            DevDataMgr& operator=(DevDataMgr &&) =delete;
    };
}
#endif
