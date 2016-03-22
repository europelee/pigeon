/**
 * @file httpclient.h
 * @brief http client wrapper for libcurl http
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-01-28
 */

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include<string>
#include <atomic> 
#include<memory>
#include<curl/curl.h>
#include "httpmsglistener.h"

namespace pigeon {

    class HTTPClient {

        public:
            HTTPClient();
            ~HTTPClient();
            void get(const std::string &url, std::shared_ptr<HttpMsgListener> ptLi);
            void post(const std::string &url, const std::string &reqBody, std::shared_ptr<HttpMsgListener> ptLi);
            std::shared_ptr<HttpMsgListener> getSharedPtrofMsgListener();
        private:
            void parseHeader(void * buffer, size_t size, size_t nmemb, HttpHeadInfo & hInfo);
            void getHelper(const std::string &url, std::shared_ptr<HttpMsgListener> ptLi);
            void postHelper(const std::string &url, const std::string &reqBody, std::shared_ptr<HttpMsgListener> ptLi);
            static char *parse_filename(const char *ptr, size_t len);
            static size_t httpRspHeader(void *buffer, size_t size, size_t nmemb, void *stream);
            static size_t httpRspCallBack(void *buffer, size_t size, size_t nmemb, void *stream);
        private:
            /** Non-copyable */
            HTTPClient(HTTPClient &&) =delete;
            HTTPClient& operator=(HTTPClient &&) =delete;
            HTTPClient(const HTTPClient&) =delete;
            HTTPClient& operator=(const HTTPClient&) =delete;

        private:

            class HTTPGlobalIni {
                public:
                    HTTPGlobalIni() {
                        curl_global_init(CURL_GLOBAL_ALL);
                    }
                    ~HTTPGlobalIni() {
                        curl_global_cleanup();
                    }

            };

        private:
            std::shared_ptr<HttpMsgListener>   mPtMsgListener;
            static HTTPGlobalIni mCurlGlobalInit;
    };
}
#endif
