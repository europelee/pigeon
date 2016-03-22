/**
 * @file ZmqEnd.h
 * @brief basic communication middleware of services on pigeon
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-16
 */
#ifndef _ZMQEND_H
#define _ZMQEND_H
#include <thread>
#include <atomic>
#include <memory>
#include "zmq.hpp"
#include "ZmqMsgListener.h"
#include "binarysemaphore.h"

#ifdef __cplusplus
    extern "C" {
#endif

#include "shm_comm.h"

#ifdef __cplusplus        
    }
#endif

namespace pigeon {
    enum class SockAction {BIND, CONNECT};
    enum class ZmqSockType {
        ROUTER_T = ZMQ_ROUTER,
        FRONTEND_T = ZMQ_DEALER,
        BACKEND_T = ZMQ_DEALER,
        QUERY_T   = ZMQ_DEALER
    };
    
    typedef struct repInfo {
        char * mPtrCliId;
        char * mPtrPayLoad;
    }repInfo;

    class ZmqEnd {
        public:
            ZmqEnd(const std::string& uri);
            ~ZmqEnd();
            void setMsgListener(std::shared_ptr<ZmqMsgListener> ptLi);
            int startMsgLoop();
            int finMsgLoop();
            int inputBackEndMsg(const std::string & cliId, const std::string & payload);
            int inputQueryRet(const std::string & cliId, const std::string & payload);

        private:
            static const int   mZmqFrameNum;
            static const std::string mQuerySockID;
            std::string mExternUri;
            const std::string  mInProcUri;
            BinarySemaphore  mBinSem;            
            zmq::context_t mCtx;
            std::atomic<bool> mThreadInter;
            zmq::socket_t  mFrontSock;
            zmq::socket_t  mMiddleSock;
            zmq::socket_t  mBackSock;
            zmq::socket_t  mQuerySock;
            std::unique_ptr<std::thread>    mPtrMsgRThread;
            std::unique_ptr<std::thread>    mPtrMsgDThread;
            std::shared_ptr<ZmqMsgListener> mPtrMsgListener;
            chn_comm_ctlinfo  mCommCtlInfo;
            
        private:
            void  frontThreadFunc();
            void  backThreadFunc();
            repInfo * createRepInfoObj(const std::string & cliId, const std::string & payload);
            void    releaseRepInfoObj(repInfo ** dPtRepO);
        private:
            /** Non-copyable */
            ZmqEnd() =delete;
            ZmqEnd(const ZmqEnd&) =delete;
            ZmqEnd& operator=(const ZmqEnd&) =delete;
            ZmqEnd(ZmqEnd &&) =delete;
            ZmqEnd& operator=(ZmqEnd &&) =delete;
    };

}
#endif
