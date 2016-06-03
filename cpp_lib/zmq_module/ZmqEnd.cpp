/**
 * @file ZmqEnd.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2015-12-18
 */
#include <array>
#include <iostream>
#include <stdlib.h>
#include "easylogging++.h"
#include "ZmqEnd.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace pigeon {

    const int ZmqEnd::mZmqFrameNum = 2;
    const std::string ZmqEnd::mQuerySockID = "QueryEndIdentity";
    ZmqEnd::ZmqEnd(const std::string& uri):mExternUri(uri), mInProcUri("inproc://BackEnd"), mCtx(1), mThreadInter(false),\
                                           mFrontSock(mCtx, static_cast<int>(ZmqSockType::ROUTER_T)), \
                                           mMiddleSock(mCtx, static_cast<int>(ZmqSockType::FRONTEND_T)), \
                                           mBackSock(mCtx, static_cast<int>(ZmqSockType::BACKEND_T)), \
                                           mQuerySock(mCtx, static_cast<int>(ZmqSockType::QUERY_T)), \
                                           mPtrMsgRThread(nullptr), mPtrMsgDThread(nullptr), mPtrMsgListener(nullptr){
                                               int ret = init_memqueue(1024*1024, &mCommCtlInfo, STREAM_IN_DIRECT);
                                               assert(ret==0);
                                           }

    ZmqEnd::~ZmqEnd() {
        LOG(TRACE) << "ZmqEnd destructor";
        int ret = fini_memqueue(1024*1024, &mCommCtlInfo, STREAM_IN_DIRECT); 
        assert(ret==0);
    }

    void ZmqEnd::setMsgListener(std::shared_ptr<ZmqMsgListener> ptLi) {
        mPtrMsgListener = ptLi; 
    }

    repInfo * ZmqEnd::createRepInfoObj(const std::string &cliId, const std::string & payload) {

        repInfo * ptRepO = (repInfo *)malloc(sizeof(repInfo));
        ptRepO->mPtrCliId = (char *)malloc(cliId.size()+1);
        ptRepO->mPtrPayLoad = (char *)malloc(payload.size()+1);
        strcpy(ptRepO->mPtrCliId, cliId.c_str());
        strcpy(ptRepO->mPtrPayLoad, payload.c_str());
        return ptRepO;
    }

    void ZmqEnd::releaseRepInfoObj(repInfo ** dPtRepO) {
        if (NULL != dPtRepO && NULL != *dPtRepO) {
            repInfo * ptRepO = *dPtRepO;
            free(ptRepO->mPtrCliId);
            ptRepO->mPtrCliId = NULL;
            free(ptRepO->mPtrPayLoad);
            ptRepO->mPtrPayLoad = NULL;
            free(ptRepO);
            ptRepO = NULL;
        }
    }

    void ZmqEnd::frontThreadFunc() {

        LOG(INFO)<<"frontThreadFunc start";

        mFrontSock.bind(mExternUri);
        mMiddleSock.bind(mInProcUri);

        zmq::pollitem_t items [] = {
            { (void *)mFrontSock, 0, ZMQ_POLLIN, 0 },
            { (void *)mMiddleSock, 0, ZMQ_POLLIN, 0 }
        };

        std::array<std::string, mZmqFrameNum> msgArray;


        while(!mThreadInter) {
            //std::cout<<"wait msg"<<std::endl;
            int more; // Multipart detection
            zmq::poll (items, sizeof(items)/sizeof(items[0]), 3000);
            if (items [0].revents & ZMQ_POLLIN) {
                int iFrame = 0;
                while (true) {
                    // Process all parts of the message
                    zmq::message_t message;
                    mFrontSock.recv(&message);

                    size_t more_size = sizeof (more);
                    mFrontSock.getsockopt (ZMQ_RCVMORE, &more, &more_size);

                    //debug
                    
                    LOG(DEBUG)<<"debug:"<<std::string(static_cast<char*>(message.data()), message.size());
                    if (iFrame >= mZmqFrameNum) {
                        LOG(ERROR)<<"error: frame num > "<<mZmqFrameNum;
                        if (nullptr != mPtrMsgListener.get()) {
                            mPtrMsgListener.get()->onError(0, std::string("err: framenum > "+mZmqFrameNum));
                        }
                        break;
                    }

                    msgArray[iFrame] = std::string(static_cast<char*>(message.data()), message.size());
                    iFrame++;
                    if (!more)
                        break; // Last message part
                }
                
                //judge msg from queryend or user_portal
                if (msgArray[0] == mQuerySockID) {
                    //parse json
                    
                    rapidjson::Document d;
                    d.Parse(msgArray[1].c_str());
                    rapidjson::Value& cliId = d["clientID"];
                    rapidjson::Value& payload = d["payload"];
                    
                    int cliIdSize = strlen(cliId.GetString());
                    int payloadSize = strlen(payload.GetString());
                    zmq::message_t outCliIdMsg(cliIdSize);
                    memcpy ((void *)outCliIdMsg.data(), cliId.GetString(), cliIdSize);
                    mFrontSock.send(outCliIdMsg, ZMQ_SNDMORE);

                    zmq::message_t outPayloadMsg(payloadSize);
                    memcpy((void *)outPayloadMsg.data(), payload.GetString(), payloadSize);
                    mFrontSock.send(outPayloadMsg, 0);
                }
                else {                
                // call ZmqMsgListener function
                if (nullptr != mPtrMsgListener.get()) {
                    mPtrMsgListener.get()->onSuccess(msgArray[0], msgArray[1]);
                } 
                }
            }//if (items [0].revents & ZMQ_POLLIN) 

            if (items[1].revents & ZMQ_POLLIN) {
                while (1) {
                    // Process all parts of the message
                    zmq::message_t message;
                    mMiddleSock.recv(&message);
                    size_t more_size = sizeof (more);
                    mMiddleSock.getsockopt (ZMQ_RCVMORE, &more, &more_size);

                    //std::string str(static_cast<char*>(message.data()), message.size());
                    //std::cout<<"dealer $$:"<<str<<std::endl;
                    mFrontSock.send(message, more!=0?ZMQ_SNDMORE:0);

                    if (!more)
                        break; // Last message part
                } 
            }

        }//while(!mThreadInter)

        LOG(INFO)<<"frontThreadFunc end";
    }

    void ZmqEnd::backThreadFunc() {
        LOG(INFO)<<"backThreadFunc start";

        mBackSock.connect(mInProcUri);

        while(!mThreadInter) {
            /* wait*/
            int wRet = mBinSem.timeWait(3);
            if (wRet == -1) {
                if (errno != ETIMEDOUT)
                    LOG(ERROR)<<strerror(errno);
                continue;
            }
            /*get msg from queue*/
            repInfo * ptRepO = nullptr;
            int ret = shm_read(&mCommCtlInfo, (void *)&ptRepO, sizeof(void *), STREAM_IN_DIRECT);
            if (SHM_OPT_FAIL == ret)
            {
                LOG(ERROR)<<"shm_read fail";
                continue;
            }
            /*send msg with zmq*/
            int cliIdSize = strlen(ptRepO->mPtrCliId);
            int payloadSize = strlen(ptRepO->mPtrPayLoad);
            zmq::message_t outCliIdMsg(cliIdSize);
            memcpy ((void *)outCliIdMsg.data(), ptRepO->mPtrCliId, cliIdSize);
            mBackSock.send(outCliIdMsg, ZMQ_SNDMORE);

            zmq::message_t outPayloadMsg(payloadSize);
            memcpy((void *)outPayloadMsg.data(), ptRepO->mPtrPayLoad, payloadSize);
            mBackSock.send(outPayloadMsg, 0);

            releaseRepInfoObj(&ptRepO);
            ptRepO = nullptr;
        }

        LOG(INFO)<<"backThreadFunc end";    
    }

    int ZmqEnd::startMsgLoop() {

        std::unique_ptr<std::thread> tmp1(new std::thread(std::mem_fn(&ZmqEnd::frontThreadFunc), this));
        mPtrMsgRThread = std::move(tmp1);

        std::unique_ptr<std::thread> tmp2(new std::thread(std::mem_fn(&ZmqEnd::backThreadFunc), this));
        mPtrMsgDThread = std::move(tmp2);
       
       mQuerySock.setsockopt(ZMQ_IDENTITY, mQuerySockID.c_str(), mQuerySockID.size()); 
       mQuerySock.connect(mExternUri);

        return 0;    
    }

    int ZmqEnd::finMsgLoop() {

        mThreadInter = true;
        mPtrMsgRThread->join();
        mPtrMsgDThread->join();
        LOG(INFO)<<"finMsgLoop end";
        return 0;

    }


    int ZmqEnd::inputBackEndMsg(const std::string & cliId, const std::string & payload) {
        int iRet = 0;
        /* put cliid&payload into queue*/
        repInfo * ptRepO = createRepInfoObj(cliId, payload);
        int ret = shm_write(&mCommCtlInfo, (void *)&ptRepO, sizeof(void *), STREAM_IN_DIRECT);
        if (SHM_OPT_FAIL == ret)
        {
            releaseRepInfoObj(&ptRepO);
            LOG(ERROR)<<"shm_write fail";
            iRet = -1;
            return iRet;
        }
        /* signal BackEndThread*/
        mBinSem.post();
        return 0;
    }

    int ZmqEnd::inputQueryRet(const std::string & cliId, const std::string & payload) {
        int iRet = 0;

        /*send msg with zmq*/
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);

        writer.StartObject();
        writer.String("clientID");
        writer.String(cliId.c_str());
        writer.String("payload");
        writer.String(payload.c_str());
        writer.EndObject();

        const std::string & tmp = s.GetString();
        LOG(DEBUG)<<"input debug:"<<tmp;
        int psize = tmp.size();
        zmq::message_t outPayloadMsg(psize);
        memcpy((void *)outPayloadMsg.data(), tmp.c_str(), psize);
        mQuerySock.send(outPayloadMsg, 0);
        
        return iRet;
    }
}
