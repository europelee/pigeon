/**
 * @file DataCollectEnd.h
 * @brief collect data (eg: agent status, devices's propes) to pigeon
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-05
 */
#ifndef _DATACOLLECTEND_H
#define _DATACOLLECTEND_H

#include <list>
#include <iostream>
#include <thread>
#include <functional>
#include <memory>
#include "easylogging++.h"
#include "GatewayPropSon.h"
#include "ISCRule.h"
#include "DataCollectInfo.h"

class DataCollectEnd {

    public:
        DataCollectEnd(const std::string &gwid): mGwId(gwid), mPropSon(new pigeon::GatewayPropSon()) {

        }

        ~DataCollectEnd() {
            LOG(TRACE)<<"DataCollectEnd destructor";
            clearDCList();
        }
        
        const std::list<DataCollectInfo *> & startCollection() {
            clearDCList();
            mPropSon->setOnLine(true);

            std::string topic = pigeon::ISCRule::setGwPropTopic(mGwId);
            DataCollectInfo *info = new DataCollectInfo(topic, mPropSon->getPropJson());
            mDCIList.push_back(info);
            return mDCIList;
        }
   
        const std::list<DataCollectInfo *> & startDevDataCollection(const std::string & devId, const std::string & devData) {
            clearDCList();
            std::string topic = pigeon::ISCRule::setGwDevDataTopic(mGwId, devId);            
            DataCollectInfo *info = new DataCollectInfo(topic, devData);
            mDCIList.push_back(info);
            return mDCIList;
        } 
    private:
        void clearDCList() {
            std::list<DataCollectInfo *>::const_iterator it;
            for (it = mDCIList.begin(); it != mDCIList.end(); ++it) {
                DataCollectInfo *info = *it;
                delete info;
                info = NULL;
            }
            mDCIList.clear();
        }
    private:
        std::list<DataCollectInfo *> mDCIList;
        std::string mGwId;
        std::unique_ptr<pigeon::GatewayPropSon> mPropSon; 
    private:
        /** Non-copyable */
        DataCollectEnd() =delete;
        DataCollectEnd(const DataCollectEnd&) =delete;
        DataCollectEnd& operator=(const DataCollectEnd&) =delete;
        DataCollectEnd(DataCollectEnd &&) =delete;
        DataCollectEnd& operator=(DataCollectEnd &&) =delete;
};
#endif
