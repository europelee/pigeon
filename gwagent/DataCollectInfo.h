/**
 * @file DataCollectInfo.h
 * @brief used by msgprocessend and datacollectend
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-08
 */
#ifndef _DATACOLLECT_INFO_H
#define _DATACOLLECT_INFO_H

#include <string>

class DataCollectInfo {
    public:
        DataCollectInfo(const std::string & topic, const std::string & payload):mTopic(topic), mPayLoad(payload) {
        }
        std::string mTopic;
        std::string mPayLoad;
};

#endif
