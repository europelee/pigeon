/**
 * @file httpmsglistener.h
 * @brief callback for processing http response
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-01-29
 */

#ifndef _HTTPMSG_LISTENER_H
#define _HTTPMSG_LISTENER_H

#include<string>
#include "httpreqerr.h"
#include "httpheadinfo.h"

namespace pigeon {

    class HttpMsgListener {
        public:
            virtual void onHead(const HttpHeadInfo & hInfo)=0;            
            virtual void onData(void *buffer, size_t len)=0;
            virtual void onFinish()=0;
            virtual void onError(HttpReqErr errCode, const std::string & errInfo)=0;
    };
}
#endif

