/**
 * @file QueryInterface.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-09
 */
#ifndef _QUERYINTERFACE_H
#define _QUERYINTERFACE_H

#include <string>
#include <functional>

typedef std::function<void(const std::string &)> QFuncObject;
typedef std::function<void(const std::string &, int , const std::string &)> QCacheFuncObject;

class QueryInterface {
    public:
        virtual void queryCallback(const std::string &cliId, int seq, const std::string &gwid, const std::string & ret) = 0;
};

#endif
