/**
 * @file DevDataMgr.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-05-23
 */
#include "base.h"
#include "ISCRule.h"
#include "DevDataMgr.h"

namespace pigeon {
    DevDataMgr::DevDataMgr(const std::string & dbUri, const std::string & devDBName, const std::string & devColName) : mMongoCxxInst{}, mMongoCli{mongocxx::uri{dbUri}}{
        mDevDataCollection = mMongoCli[devDBName][devColName];
    }

    DevDataMgr::~DevDataMgr() {

    }

    bool DevDataMgr::saveDevData(const std::string & gwid, const std::string & devtype, const std::string & data) {
        bool ret = false;

        bsoncxx::builder::stream::document filter_builder, update_builder;
        filter_builder << pigeon::ISCRule::iot_gwid_dbtag << gwid << pigeon::ISCRule::iot_devtype_dbtag << devtype;
        bsoncxx::stdx::string_view json(data);
        bsoncxx::array::value dataValue = bsoncxx::from_jsonarray(json);

        using bsoncxx::builder::stream::open_document;
        using bsoncxx::builder::stream::close_document;

        update_builder<<"$set"<<open_document<< pigeon::ISCRule::iot_devinstlist_dbtag
            <<bsoncxx::types::b_array{dataValue.view()} <<close_document;
        mongocxx::options::update options;
        options.upsert(true);

        try {
            auto result = mDevDataCollection.update_one(filter_builder.view(), update_builder.view(), options);

            if (result->matched_count() !=0 || result->upserted_id()) {
                ret = true;
            }
        }
        catch (const mongocxx::exception & e) {

            LOG(FATAL)<<e.what();
            ret = false;
            return ret;
        }
        
        return ret;
    }
}
