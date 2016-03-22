/**
 * @file ZmqMsgListener.h
 * @brief as a interface for getting msg from zmq_module
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-17
 */

#ifndef _ZMQMSGLISTENER_H
#define _ZMQMSGLISTENER_H

#include<string>

namespace pigeon {

    class ZmqMsgListener {
        public:
            virtual void onSuccess(const std::string & cliID, const std::string & cliMsg)=0;
            virtual void onError(int errCode, const std::string & err)=0;
    };
}
#endif
