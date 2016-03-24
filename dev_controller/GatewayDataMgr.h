/**
 * @file GatewayDataMgr.h
 * @brief parsing and store data from gateway(include gw-status and devices)
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-28
 */

/**
 * todo:
 * operation on redis need to be reconstructed, 
 * for atomic, should use multi/exec and watch  or redis's lua script even through hiredis-aysnc
 * europelee
 * 2016-03-22
 */
#ifndef _GATEWAY_DATASTORE_H
#define _GATEWAY_DATASTORE_H
#include <list>
#include <string>
#include <thread>
#include <map>
#include <functional>
#include "QueryInterface.h"

struct aeEventLoop;
struct redisAsyncContext;

namespace pigeon {
    class GatewayDataMgr {

        public:
            GatewayDataMgr(const std::string & redisIPAddr, int port);
            ~GatewayDataMgr();
            int startRedisCliProcLoop();
            void stopRedisCliProc();
            void cacheUpRouterInfo(const std::string & optId, const std::string & cliId, int uSeq, const std::string & gwId);
            void getUpRouterInfo(const std::string & optId, QCacheFuncObject * ptFuncObj);
            void getGatewayProp(const std::list<std::string> &gwidList, QFuncObject *ptFuncObj);
            void getGatewayProp(const std::string & gwid, QFuncObject * ptFuncObj);            
            void saveGatewayProp(const std::string & gwid, const std::string & prop, bool bScript=true);
        private:
            class GatewayProp {
                public:
                    GatewayProp(const char * gwid, const char * prop):mGwId(gwid), mGwProp(prop) {
                    }

                    ~GatewayProp() {
                    }

                public:
                    std::string mGwId;
                    std::string mGwProp;

            };
            
            class UpCacheRInfo {
                public:
                    static const char * cliID_tag;
                    static const char * seq_tag;
                    static const char * gwid_tag;
                public:
                    UpCacheRInfo(const std::string & optId, QCacheFuncObject * pf):mOptId(optId), mQFunc(pf){
                    }
                    ~UpCacheRInfo() {
                        delete mQFunc;
                        mQFunc = NULL;
                    }
                    const std::string & getOptId() const {
                        return mOptId;
                    }

                    QCacheFuncObject * getQCacheFuncObj() {
                        return mQFunc;
                    }
                private:
                    std::string mOptId;
                    QCacheFuncObject *mQFunc;
            };
            
            std::string mRedisSvrIPAddr;
            int mPort;
            redisAsyncContext *mPtRedisAC;
            aeEventLoop *mPtEventLoop;
            std::map<std::string, std::string> mScriptSha1Map;
        private:
            void loopThread();
            static void connectCallback(const redisAsyncContext *c, int status);
            static void disconnectCallback(const redisAsyncContext *c, int status);
            static void quitConnCallBack(redisAsyncContext *c, void *r, void *privdata); 
            static void checkGWIDCallback(redisAsyncContext *c, void *r, void *privdata);
            static void updateGWPropCallback(redisAsyncContext *c, void *r, void *privdata); 
            static void updateGWPropCallbackExScript(redisAsyncContext *c, void *r, void *privdata); 
            static void queryGWPropCallback(redisAsyncContext *c, void *r, void *privdata);
            static void multiCallback(redisAsyncContext *c, void *r, void *privdata);
            static void execCallback(redisAsyncContext *c, void *r, void *privdata);
            static void cacheUpRInfoCallback(redisAsyncContext *c, void *r, void *privdata);
            static void getUpRInfoCallback(redisAsyncContext *c, void *r, void *privdata);
            static void expireUpRCacheCallback(redisAsyncContext *c, void *r, void *privdata);
            static void delUpRCacheCallback(redisAsyncContext *c, void *r, void *privdata);
        
        private:
            static const std::string & saveGatewayProp_script;

        private:
            static bool readScript(const std::string & scriptFilePath, std::string & scriptStream);
            void sha1sumScript();
            bool sha1sumScriptHelper(const std::string & scriptFilePath, std::string & sha1val);

        private:
            /** Non-copyable */
            GatewayDataMgr(const GatewayDataMgr&) =delete;
            GatewayDataMgr& operator=(const GatewayDataMgr&) =delete;
            GatewayDataMgr(GatewayDataMgr &&) =delete;
            GatewayDataMgr& operator=(GatewayDataMgr &&) =delete;
    };
}

#endif
