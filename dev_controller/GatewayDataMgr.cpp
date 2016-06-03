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
#include <fstream>
#include <sstream> 
#include <ISCRule.h>
#include "base.h"
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

    const std::string & GatewayDataMgr::saveGatewayProp_script = "lua/saveGatewayProp.lua";
    const std::string & GatewayDataMgr::cacheUpRepSub_script = "lua/cacheUPRepSub.lua";
    const std::string & GatewayDataMgr::delUpRepSub_script = "lua/delUPRepSub.lua";

    const char * GatewayDataMgr::UpCacheRInfo::cliID_tag = "cliId";
    const char * GatewayDataMgr::UpCacheRInfo::seq_tag = "seq";
    const char * GatewayDataMgr::UpCacheRInfo::gwid_tag = "gwid";

    GatewayDataMgr::GatewayDataMgr(const std::string & redisIPAddr, int port) \
        :mRedisSvrIPAddr(redisIPAddr), mPort(port) {

        }

    GatewayDataMgr::~GatewayDataMgr() {
        mScriptSha1Map.clear();
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
        
        //init redis script sha1
        sha1sumScript();

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
        if (reply == NULL) {
            printf("[%s] updateGWPropCallback: reply null\n", (char*)privdata);
            if (NULL != privdata) {
                free(privdata);
                privdata = NULL;
            }
            return;
        }

        printf("[%s] updateGWPropCallback: %s\n", (char*)privdata, reply->str);

        if (NULL != privdata) {
            free(privdata);
            privdata = NULL;
        }

    }

    void GatewayDataMgr::updateGWPropCallbackExScript(redisAsyncContext *c, void *r, void *privdata) {
    
        redisReply *reply = (redisReply *)r;
        GatewayProp * gp = (GatewayProp *)privdata;
        
        if (!gp) {
            printf("happen exception updateGWPropCallbackExScript privdata is null\n");
            return;
        }

        if (reply == NULL) {
            
            printf("[%s] updateGWPropCallback: reply null\n", gp->mGwId.c_str());
            delete gp;
            gp = NULL;
            return;
        }

        printf("[%s] updateGWPropCallback: %s\n", gp->mGwId.c_str(), reply->str);
        if (reply->type == REDIS_REPLY_ERROR) {
            if (NULL != strstr(reply->str, "NOSCRIPT No matching script")) {
                //USE EVAL
                printf("redis-server not cache, we need eval\n");
                std::string scriptStream = "";
                bool ret = readScript(saveGatewayProp_script, scriptStream);

                if (!ret) {
                    delete gp;
                    gp = NULL;
                    return;
                }

                char gwPropKey[128];
                snprintf(gwPropKey, 127, "%s:%s:%s", ISCRule::iot_gw_dbtag.c_str(), gp->mGwId.c_str(), \
                        ISCRule::iot_gwprop_dbtag.c_str());
                gwPropKey[127] = 0;
                redisAsyncCommand(c, updateGWPropCallbackExScript, gp, "eval %s %d %s %s %s %s", \
                        scriptStream.c_str(), 2, ISCRule::iot_gw_dbtag.c_str(), gwPropKey, gp->mGwId.c_str(), gp->mGwProp.c_str());
                return;
            }
        }
        delete gp;
        gp = NULL;
    }
    
    void GatewayDataMgr::delUpRepSubCallbackExScript(redisAsyncContext *c, void *r, void *privdata) {
    
        MemMap * ptMap = (MemMap *)privdata;
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            printf("[%s] delUpRepSubCallbackExScript: reply null\n", ptMap!=NULL?ptMap->mKey.c_str():NULL);
            if (NULL != ptMap) {
                delete ptMap;
                ptMap = NULL;
            }
            return;
        }

        
        if (!ptMap) {
            printf("happen exception delUpRepSubCallbackExScript privdata is null\n");
            return;
        }

        printf("[%s] delUpRepSubCallbackExScript: %lld\n", ptMap->mKey.c_str(), reply->integer);

        if (reply->type == REDIS_REPLY_ERROR) {
            if (NULL != strstr(reply->str, "NOSCRIPT No matching script")) {
                //USE EVAL
                printf("redis-server not cache, we need eval\n");
                std::string scriptStream = "";
                bool ret = readScript(delUpRepSub_script, scriptStream);

                if (!ret) {
                    delete ptMap;
                    ptMap = NULL;
                    return;
                }

                redisAsyncCommand(c, delUpRepSubCallbackExScript, ptMap, "eval %s %d %s %s", \
                        scriptStream.c_str(), 1, ptMap->mKey.c_str(), ptMap->mValue.c_str());
                return;
            }
        }
        delete ptMap;
        ptMap = NULL;
    }

    void GatewayDataMgr::cacheUpRepSubCallbackExScript(redisAsyncContext *c, void *r, void *privdata) {
    
        MemMap * ptMap = (MemMap *)privdata;
        redisReply *reply = (redisReply *)r;
        if (reply == NULL) {
            printf("[%s] cacheUpRepSubCallbackExScript: reply null\n", (char*)privdata);
            if (NULL != ptMap) {
                delete ptMap;
                ptMap = NULL;
            }
            return;
        }

        
        if (!ptMap) {
            printf("happen exception cacheUpRepSubCallbackExScript privdata is null\n");
            return;
        }

        printf("[%s] cacheUpRepSubCallbackExScript: %lld\n", ptMap->mKey.c_str(), reply->integer);

        if (reply->type == REDIS_REPLY_ERROR) {
            if (NULL != strstr(reply->str, "NOSCRIPT No matching script")) {
                //USE EVAL
                printf("redis-server not cache, we need eval\n");
                std::string scriptStream = "";
                bool ret = readScript(cacheUpRepSub_script, scriptStream);

                if (!ret) {
                    delete ptMap;
                    ptMap = NULL;
                    return;
                }

                redisAsyncCommand(c, cacheUpRepSubCallbackExScript, ptMap, "eval %s %d %s %s", \
                        scriptStream.c_str(), 1, ptMap->mKey.c_str(), ptMap->mValue.c_str());
                return;
            }
        }
        delete ptMap;
        ptMap = NULL;
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
            sprintf(tmp, "%s:%s:%s", ISCRule::iot_gw_dbtag.c_str(), ptProp->mGwId.c_str(), ISCRule::iot_gwprop_dbtag.c_str());
            ptUqPropSon->parsePropJson(ptProp->mGwProp.c_str());

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

    void GatewayDataMgr::saveGatewayProp(const std::string & gwid, const std::string &prop, bool bScript) {
        
        GatewayProp *PtProp = new GatewayProp(gwid.c_str(), prop.c_str()); 
        
        if (true == bScript) {
            std::map<std::string,std::string>::iterator it;
            it = mScriptSha1Map.find(saveGatewayProp_script);
            if (it == mScriptSha1Map.end()) {
                printf("not find %s sha1val\n", saveGatewayProp_script.c_str());
                delete PtProp;
                PtProp = NULL;
                return;
            }
            
            printf("sha1: [%s]\n", it->second.c_str());

            char gwPropKey[128];
            snprintf(gwPropKey, 127, "%s:%s:%s", ISCRule::iot_gw_dbtag.c_str(), PtProp->mGwId.c_str(), \
                    ISCRule::iot_gwprop_dbtag.c_str());
            redisAsyncCommand(mPtRedisAC, updateGWPropCallbackExScript, PtProp, "evalsha %s %d %s %s %s %s", \
                    it->second.c_str(), 2, ISCRule::iot_gw_dbtag.c_str(), gwPropKey, PtProp->mGwId.c_str(), PtProp->mGwProp.c_str());
            return;
        }
        
        redisAsyncCommand(mPtRedisAC, checkGWIDCallback, PtProp, "sismember  %s %s", ISCRule::iot_gw_dbtag.c_str(), gwid.c_str());
    }


    void GatewayDataMgr::saveDeviceData(const std::string & gwid, const std::string & devtype, const std::string & data, bool bScript) {
    
    }
    
    void GatewayDataMgr::cacheUpRouterInfo(const std::string & optId, const std::string & cliId, int uSeq, const std::string & gwId) {

        redisAsyncCommand(mPtRedisAC, cacheUpRInfoCallback, (void *)optId.c_str(), "hmset %s %s %s %s %d %s %s", optId.c_str(), UpCacheRInfo::cliID_tag, cliId.c_str(), UpCacheRInfo::seq_tag, uSeq, UpCacheRInfo::gwid_tag, gwId.c_str());
    }

    void GatewayDataMgr::getUpRouterInfo(const std::string & optId, QCacheFuncObject * ptFuncObj) {
        UpCacheRInfo * info = new UpCacheRInfo(optId, ptFuncObj); 
        redisAsyncCommand(mPtRedisAC, getUpRInfoCallback, info, "hgetall %s", optId.c_str());
    }


    bool GatewayDataMgr::readScript(const std::string & scriptFilePath, std::string & scriptStream) {

        std::ifstream iscript(scriptFilePath);

        if (false == iscript.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << iscript.rdbuf();
        scriptStream = buffer.str();
        LOG(DEBUG)<<scriptStream;
        iscript.close();
        iscript.clear();
        return true;
    }

    void GatewayDataMgr::sha1sumScript() {
        std::string sha1val = "";
        bool ret = false;
        ret = sha1sumScriptHelper(saveGatewayProp_script, sha1val);
        
        if (true == ret) {
            mScriptSha1Map.insert(std::make_pair<std::string, std::string>(std::string(saveGatewayProp_script), std::string(sha1val)));
        }

        ret = sha1sumScriptHelper(cacheUpRepSub_script, sha1val);
        
        if (true == ret) {
            mScriptSha1Map.insert(std::make_pair<std::string, std::string>(std::string(cacheUpRepSub_script), std::string(sha1val)));
        }

        ret = sha1sumScriptHelper(delUpRepSub_script, sha1val);
        
        if (true == ret) {
            mScriptSha1Map.insert(std::make_pair<std::string, std::string>(std::string(delUpRepSub_script), std::string(sha1val)));
        }
    }
    
    bool GatewayDataMgr::sha1sumScriptHelper(const std::string & scriptFilePath, std::string & sha1val) {
    
        char buf[256];
        sprintf(buf, "sha1sum %s|cut -d\" \" -f1", scriptFilePath.c_str());
        FILE *pp;
        if( (pp = popen(buf, "r")) == NULL )
        {
            printf("popen() error for %s!/n", buf);
            return false;
        }
        while(fgets(buf, sizeof(buf), pp))
        {
            if ( strlen(buf) > 1 && buf[strlen(buf)-1] == '\n') {
                buf[strlen(buf)-1] = 0;
            }
            sha1val = buf;
        }
        pclose(pp);
        return true;
    }


    void GatewayDataMgr::cacheUpRepSub(const std::string & subKey, const std::string & subTopic, bool bScript) {

        MemMap * ptMap = new MemMap(subKey, subTopic); 
        if (bScript) {
        
            std::map<std::string,std::string>::iterator it;
            it = mScriptSha1Map.find(cacheUpRepSub_script);
            if (it == mScriptSha1Map.end()) {
                printf("not find %s sha1val\n", cacheUpRepSub_script.c_str());
                delete ptMap;
                ptMap = NULL;
                return;
            }
            
            redisAsyncCommand(mPtRedisAC, cacheUpRepSubCallbackExScript, ptMap, "evalsha %s %d %s %s", \
                    it->second.c_str(), 1, subKey.c_str(), subTopic.c_str());
            return;
        }
    }

    void GatewayDataMgr::delUpRepSub(const std::string & subKey, const std::string & subTopic, bool bScript) {
        MemMap * ptMap = new MemMap(subKey, subTopic); 
        if (bScript) {
        
            std::map<std::string,std::string>::iterator it;
            it = mScriptSha1Map.find(delUpRepSub_script);
            if (it == mScriptSha1Map.end()) {
                printf("not find %s sha1val\n", delUpRepSub_script.c_str());
                delete ptMap;
                ptMap = NULL;
                return;
            }
            
            redisAsyncCommand(mPtRedisAC, delUpRepSubCallbackExScript, ptMap, "evalsha %s %d %s %s", \
                    it->second.c_str(), 1, subKey.c_str(), subTopic.c_str());
            return;
        }
    }
}
