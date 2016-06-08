/**
 * @file gwRegListener.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-16
 */
#ifndef _GWREG_LISTENER_H
#define _GWREG_LISTENER_H

#include <string.h>
#include <string>
#include <iostream>
#include "easylogging++.h"
#include "httpmsglistener.h"

namespace pigeon {

    class gwRegListener : public pigeon::HttpMsgListener {

        virtual void onHead(const pigeon::HttpHeadInfo & hInfo) override {

        }

        virtual void onData(void * buffer, size_t len) override {

            char * ptrData = (char *)malloc(len+1);
            memcpy(ptrData, buffer, len);
            ptrData[len] = 0;
            LOG(DEBUG)<<"onData:"<<ptrData;        
            free(ptrData);
        }

        virtual void onFinish() override {
            LOG(INFO)<<"finish gwagent reg-request";    
        }

        virtual void onError(HttpReqErr errCode, const std::string & errInfo) override {

        }
    };
}

#endif
