#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <thread>
#include<functional>

#include "rapidjson/document.h"
#include "GatewayPropSon.h"
#ifdef __cplusplus
    extern "C"{
#endif

#include <hiredis.h>
#include <async.h>
#include <adapters/ae.h>
#ifdef __cplusplus
           }
#endif
/* Put event loop in the global scope, so it can be explicitly stopped */
static volatile bool stop = false;
static aeEventLoop *loop;
static char id[64] = {0};
void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; ++i) {
            
            printf("argv[%s]: %s\n", (char*)privdata, reply->element[i]->str);
            printf("argv[%s]: %lld\n", (char*)privdata, reply->element[i]->integer);
        }
    }
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);
    printf("argv[%s]: %lld\n", (char*)privdata, reply->integer);

}


void loopCheckGWIDCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    printf("[%s]: %lld\n", (char*)privdata, reply->integer);
    //redisAsyncDisconnect(c);
}

void updateGWStatusCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    printf("[%s]: %s\n", (char*)privdata, reply->str);

}

void checkGWIDCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    printf("[%s]: %lld\n", (char*)privdata, reply->integer);
    if (reply->integer == 1) {
        char tmp[128];
        sprintf(tmp, "iot:gateway:%s:status", id);
        char c11[64];
        strcpy(c11,  "updateGWStatus");
        redisAsyncCommand(c, updateGWStatusCallback, c11, "hmset %b %s %d %s %s", tmp, strlen(tmp), "online", 1, "teststr","string-test");
    }
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("c Error: %s\n", c->errstr);
        aeStop(loop);
        return;
    }

    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("d Error: %s\n", c->errstr);
        aeStop(loop);
        return;
    }

    printf("Disconnected...\n");
    aeStop(loop);
}

void quitConnCallBack(redisAsyncContext *c, void *r, void *privdata) {
    printf("quit");
    redisAsyncDisconnect(c);
}

void testThreadLoop(void * p) {
    static int num = 150;
    char c11[64];
    strcpy(c11, "checkGWID");
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        num--;
        if (num < 0) {
                     
            redisAsyncCommand((redisAsyncContext *)p, quitConnCallBack, c11, "quit");
            printf("exit\n");
            return;
        }
        //redisAsyncCommand((redisAsyncContext *)p, getCallback, c11, "get testint");
        //redisAsyncCommand((redisAsyncContext *)p, getCallback, c11, "ping");
        //redisAsyncCommand((redisAsyncContext *)p, checkGWIDCallback, c11, "sismember  %b %b", "iot:gateway", 11, id, strlen(id));
        char tmp[256] = {0};
        sprintf(tmp, "iot:gateway:%s:prop", id);
        redisAsyncCommand((redisAsyncContext *)p, getCallback, c11, "hgetall %s", tmp);
        num = -1;
    }
}

void testaeMainThread(void *p) {
    (void *)p;
    aeMain(loop);
    aeDeleteEventLoop(loop);
    stop = true;
    return;
}

int main (int argc, char **argv) {

    const char * ptContent = "{\"online\":1}";
    rapidjson::Document document;
    document.Parse(ptContent);
  // Iterating object members
    static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
        printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        if (itr->value.IsNumber()&&itr->value.IsInt()) {
            printf("values is %d\n", itr->value.GetInt());
        }
    }

    pigeon::GatewayPropSon gwson;
    gwson.parsePropJson(ptContent);
    std::cout<<gwson.getOnLine()<<std::endl;
    gwson.setOnLine(false);
    std::cout<<gwson.getPropJson()<<std::endl;

//    return 0;

    if (argc < 2)
        return -1;

    signal(SIGPIPE, SIG_IGN);

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    loop = aeCreateEventLoop(64);
    redisAeAttach(loop, c);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);

    //redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1], strlen(argv[argc-1]));
    strcpy(id, argv[1]);
    std::thread t(testThreadLoop, c);
    t.detach();
    //redisAsyncCommand(c, checkGWIDCallback, "checkGWID", "sismember  %b %b", "iot:gateway", 11, argv[1], strlen(argv[1]));
    
    //aeMain(loop);
//    redisAsyncFree(c);
    //aeDeleteEventLoop(loop);
    
    std::thread t2(testaeMainThread, nullptr);
    t2.detach();
    while(!stop) {
    
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    printf("end\n");
    return 0;
}

