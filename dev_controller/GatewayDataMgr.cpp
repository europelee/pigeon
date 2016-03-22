/**
 * @file GateWayDataMgr.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-29
 */
#include <stdio.h>
#include <string.h>
#include <thread>
#include <functional>
#include <memory>
#include <ISCRule.h>
#include <GatewayPropSon.h>
#include "GatewayDataMgr.h"

#ifdef __cplusplus
extern "C"{
#endif

#include <hiredis.h>
#include <async.h>
#include <adapters/ae.h>

#ifdef __cplusplus
}
#endif

namespace pigeon {
    
    const char * GatewayDataMgr::UpCacheRInfo::cliID_tag = "cliId";
    const char * GatewayDataMgr::UpCacheRInfo::seq_tag = "seq";
    const char * GatewayDataMgr::UpCacheRInfo::gwid_tag = "gwid";

    GatewayDataMgr::GatewayDataMgr(const std::string & redisIPAddr, int port) \
        :mRedisSvrIPAddr(redisIPAddr), mPort(port) {

        }

    GatewayDataMgr::~GatewayDataMgr() {

    }


    void GatewayDataMgr::connectCallback(const redisAsyncContext *c, int status) {

        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            aeStop((aeEventLoop*)c->data);
            return;
        }

        printf("Connected...\n");
    }

    void GatewayDataMgr::disconnectCallback(const redisAsyncContext *c, int status) {

        if (status != REDIS_OK) {
            printf("Error: %s\n", c->errstr);
            aeStop((aeEventLoop*)(c->data));
            return;
        }

        printf("Disconnected...\n");
        aeStop((aeEventLoop*)(c->data));
    }

    void GatewayDataMgr::quitConnCallBack(redisAsyncContext *c, void *r, void *privdata) {

        printf("quit");
        redisAsyncDisconnect(c);
    } 

    int GatewayDataMgr::startRedisCliProcLoop() {
        int iRet = 0;

        mPtRedisAC = redisAsyncConnect(mRedisSvrIPAddr.c_str(), mPort);
        if (mPtRedisAC->err) {
            /* Let *c leak for now... */
            printf("Error: %s\n", mPtRedisAC->errstr);
            iRet = -1;
            return iRet;
        }

        mPtEventLoop = aeCreateEventLoop(64);
        mPtRedisAC->data = mPtEventLoop;//used on connect/disconnect callback
        redisAeAttach(mPtEventLoop, mPtRedisAC);
        redisAsyncSetConnectCallback(mPtRedisAC, connectCallback);
        redisAsyncSetDisconnectCallback(mPtRedisAC,disconnectCallback);

        std::thread loopThread(std::mem_fn(&GatewayDataMgr::loopThread), this);
        loopThread.detach();
        return iRet;
    } 

    void GatewayDataMgr::loopThread() {
        printf("start loopThread\n");
        aeMain(mPtEventLoop);
        aeDeleteEventLoop(mPtEventLoop);
        printf("end loopThread\n");
        return;
    }


    void GatewayDataMgr::stopRedisCliProc() {
        if (NULL != mPtRedisAC) { 
            redisAsyncCommand(mPtRedisAC, quitConnCallBack, NULL, "quit");
        }
    }

    void GatewayDataMgr::delUpRCacheCallback(redisAsyncContext *c, void *r, void *privdata) {
         
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            
            printf("[%s]-delUpRCache: reply null\n", (char *)privdata);
            return;
        }

        printf("[%s]: %lld\n", "delUpRCacheCallback", reply->integer);
    }

    void GatewayDataMgr::getUpRInfoCallback(redisAsyncContext *c, void *r, void *privdata) {
    
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            printf("getUpRouterInfo reply is null\n");
            
            UpCacheRInfo* ptQf = (UpCacheRInfo *)privdata;
            delete ptQf;
            ptQf = NULL;
            return;
        }
        
        std::string cliId;
        int seq;
        std::string gwId;

        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i=i+2) {
                if (0 == strcmp(UpCacheRInfo::cliID_tag, reply->element[i]->str)) {
                    cliId = reply->element[i+1]->str;
                }
                
                if (0 == strcmp(UpCacheRInfo::seq_tag, reply->element[i]->str)) {
                    seq = atoi(reply->element[i+1]->str); 
                }
                
                if (0 == strcmp(UpCacheRInfo::gwid_tag, reply->element[i]->str)) {
                    gwId = reply->element[i+1]->str;
                }
            }
        }
        
        UpCacheRInfo * info = (UpCacheRInfo *)privdata;

        QCacheFuncObject * ptQf = info->getQCacheFuncObj();
        (*ptQf)(cliId, seq, gwId);
        
        //del cache
        redisAsyncCommand(c, delUpRCacheCallback, NULL, "del %s", info->getOptId().c_str());
        
        delete info;
        info = NULL;
    }

    void GatewayDataMgr::queryGWPropCallback(redisAsyncContext *c, void *r, void *privdata) {
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            
            QFuncObject * ptQf = (QFuncObject *)privdata;
            delete ptQf;
            ptQf = NULL;
            return;
        }

        std::unique_ptr<GatewayPropSon> ptUqPropSon(new GatewayPropSon());
        if (reply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < reply->elements; i=i+2) {
                if (0 == strcmp(ptUqPropSon->getOnLineTag(), reply->element[i]->str)) {
                    if (0 == strcmp("1", reply->element[i+1]->str)) {
                        ptUqPropSon->setOnLine(true);
                    }
                    else {
                        ptUqPropSon->setOnLine(false);
                    }
                }
            }
        }

        QFuncObject * ptQf = (QFuncObject *)privdata;
        (*ptQf)(ptUqPropSon->getPropJson());
        delete ptQf;
        ptQf = NULL;
    }

    void GatewayDataMgr::updateGWPropCallback(redisAsyncContext *c, void *r, void *privdata) {
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) return;
        printf("[%s]: %s\n", (char*)privdata, reply->str);

    }

    void GatewayDataMgr::checkGWIDCallback(redisAsyncContext *c, void *r, void *privdata) {
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            delete (GatewayProp *)privdata;
            privdata = NULL;
            return;
        }

        GatewayProp *ptProp = (GatewayProp *)privdata;
        printf("checkGWIDCallback: %lld\n", reply->integer);
        if (reply->integer == 1) {

            std::unique_ptr<GatewayPropSon> ptUqPropSon(new GatewayPropSon());
            char tmp[128];
            sprintf(tmp, "%s:%s:%s", ISCRule::iot_gw_dbtag.c_str(), ptProp->ptGwId, ISCRule::iot_gwprop_dbtag.c_str());
            ptUqPropSon->parsePropJson(ptProp->ptGwProp);

            redisAsyncCommand(c, updateGWPropCallback, NULL, "hmset %s %s %d", tmp, ptUqPropSon->getOnLineTag
                    (), ptUqPropSon->getOnLine());
        }

        delete ptProp;
        ptProp = NULL;
    }


    void GatewayDataMgr::multiCallback(redisAsyncContext *c, void *r, void *privdata) {

    }

    void GatewayDataMgr::execCallback(redisAsyncContext *c, void *r, void *privdata) {

    }

    void GatewayDataMgr::cacheUpRInfoCallback(redisAsyncContext *c, void *r, void *privdata) {
    
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            
            printf("[%s]-cacheUpRouterInfo: reply null\n", (char *)privdata);
            return;
        }

        printf("[%s]: %s\n", (char*)privdata, reply->str);
        redisAsyncCommand(c, expireUpRCacheCallback, NULL, "expire  %s %d", (char *)privdata, 30);
    }

    void GatewayDataMgr::expireUpRCacheCallback(redisAsyncContext *c, void *r, void *privdata) {
    
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            printf("[%s]: reply null\n", "expireUpRCacheCallback");
            return;
        }

        printf("[%s]: %lld\n", "expireUpRCacheCallback", reply->integer);
    }

    void GatewayDataMgr::getGatewayProp(const std::list<std::string> &gwidList, QFuncObject *ptFuncObj){

    } 

    void GatewayDataMgr::getGatewayProp(const std::string & gwid, QFuncObject * ptFuncObj) {
        //userportal would check user's gw ownship(include checking if gw registerd)
        //so here, we just get
        char tmp[128];
        sprintf(tmp, "%s:%s:%s", ISCRule::iot_gw_dbtag.c_str(), gwid.c_str(), ISCRule::iot_gwprop_dbtag.c_str());
        redisAsyncCommand(mPtRedisAC, queryGWPropCallback, ptFuncObj, "hgetall %s", tmp);
    }

    void GatewayDataMgr::saveGatewayProp(const std::string & gwid, const std::string &prop) {
        GatewayProp *PtProp = new GatewayProp(gwid.c_str(), prop.c_str()); 
        redisAsyncCommand(mPtRedisAC, checkGWIDCallback, PtProp, "sismember  %s %s", ISCRule::iot_gw_dbtag.c_str(), gwid.c_str());
    }


    void GatewayDataMgr::cacheUpRouterInfo(const std::string & optId, const std::string & cliId, int uSeq, const std::string & gwId) {
    
        redisAsyncCommand(mPtRedisAC, cacheUpRInfoCallback, (void *)optId.c_str(), "hmset %s %s %s %s %d %s %s", optId.c_str(), UpCacheRInfo::cliID_tag, cliId.c_str(), UpCacheRInfo::seq_tag, uSeq, UpCacheRInfo::gwid_tag, gwId.c_str());
    }

    void GatewayDataMgr::getUpRouterInfo(const std::string & optId, QCacheFuncObject * ptFuncObj) {
        UpCacheRInfo * info = new UpCacheRInfo(optId, ptFuncObj); 
        redisAsyncCommand(mPtRedisAC, getUpRInfoCallback, info, "hgetall %s", optId.c_str());
    }
}
