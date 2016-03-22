/**
 * @file testhttpclient.cpp
 * @brief test httpclient
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-03
 */
#include<string.h>
#include<string>
#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include "httpclient.h"
#include "httpmsglistener.h"
using namespace pigeon;

class testGetMsgListener : public HttpMsgListener {
    private:
        FILE * fp;
        std::string mFileName;
    public:
        testGetMsgListener(std::string fileName):fp(NULL), mFileName(fileName) {}
        virtual void onHead(const HttpHeadInfo & hInfo) override {
            if (hInfo.b_cb_filename) {
                mFileName = hInfo.fileName;
            }
        }
        virtual void onData(void * buffer, size_t len) override{
            if (fp==NULL) {
                fp = fopen(mFileName.c_str(), "wb");
                if (fp==NULL) {
                    std::cout<<std::string(strerror(errno));
                    return;
                }
            }
            fwrite(buffer, len, 1, fp);
        }

        virtual void onFinish() override {
            std::cout<<"finish http get request"<<std::endl;
            if (fp!=NULL) {
                fflush(fp);
                fclose(fp);
                fp = NULL;
            }    
        }

        virtual void onError(HttpReqErr errCode, const std::string & errInfo) override {
        }
};

class testMsgListener : public HttpMsgListener {

    virtual void onHead(const HttpHeadInfo & hInfo) override {
    
    }
    
    virtual void onData(void * buffer, size_t len) override {

        printf("testpost:hi write_data\n");
        char * ptrData = (char *)malloc(len+1);
        memcpy(ptrData, buffer, len);
        ptrData[len] = 0;
        fprintf(stdout, "%s\n", ptrData);        
        free(ptrData);
    }

    virtual void onFinish() override {
        std::cout<<"finish http get request"<<std::endl;    
    }

    virtual void onError(HttpReqErr errCode, const std::string & errInfo) override {
    }
};

int main() {
    std::shared_ptr<HttpMsgListener> li = std::make_shared<testGetMsgListener>("./test1-im");
    HTTPClient client;
    client.get(std::string("http://deviceportal.athenacloud.net/v0.1/iot/devices/gateway/gwid/devplug/plugid?version=0.1"), li);

    std::shared_ptr<HttpMsgListener> li1 = std::make_shared<testGetMsgListener>("./test2-im");
    HTTPClient client1;
    client1.get(std::string("http://deviceportal.athenacloud.net/v0.1/iot/devices/gateway/gwid/devplug/plugid?version=1.1"), li1);

    std::shared_ptr<HttpMsgListener> li2 = std::make_shared<testMsgListener>();
    HTTPClient client2;
    client2.post(std::string("http://deviceportal.athenacloud.net/v0.1/iot/devices/gateway"), std::string("{\"macaddr\":\"4991356210\"}"), li2);
    while(1) {
        sleep(1);
    }
    return 0;
}
