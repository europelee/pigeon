/**
 * @file httpclient.cpp
 * @brief httpclient
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-01-29
 */
#include<string.h>
#include<thread>
#include<iostream>
#include <curl/curl.h>
#include "httpclient.h"

namespace pigeon {

    HTTPClient::HTTPGlobalIni HTTPClient::mCurlGlobalInit;
    HTTPClient::HTTPClient():mPtMsgListener(nullptr) {

    }

    HTTPClient::~HTTPClient() {

    }

    std::shared_ptr<HttpMsgListener> HTTPClient::getSharedPtrofMsgListener() {
        return this->mPtMsgListener;
    }


    char *HTTPClient::parse_filename(const char *ptr, size_t len) {
        char *copy;
        char *p;
        char *q;
        char  stop = '\0';

        /* simple implementation of strndup() */
        copy = (char *)malloc(len+1);
        if(!copy)
            return NULL;
        memcpy(copy, ptr, len);
        copy[len] = '\0';

        p = copy;
        if(*p == '\'' || *p == '"') {
            /* store the starting quote */
            stop = *p;
            p++;
        }
        else
            stop = ';';

        /* if the filename contains a path, only use filename portion */
        q = strrchr(copy, '/');
        if(q) {
            p = q + 1;
            if(!*p) {
                free(copy);
                copy = NULL;
                return NULL;
            }
        }

        /* If the filename contains a backslash, only use filename portion. The idea
         *      is that even systems that don't handle backslashes as path separators
         *           probably want the path removed for convenience. */
        q = strrchr(p, '\\');
        if(q) {
            p = q + 1;
            if(!*p) {
                free(copy);
                copy = NULL;
                return NULL;
            }
        }

        /* scan for the end letter and stop there */
        for(q = p; *q; ++q) {
            if(*q == stop) {
                *q = '\0';
                break;
            }
        }

        /* make sure the file name doesn't end in \r or \n */
        q = strchr(p, '\r');
        if(q)
            *q = '\0';

        q = strchr(p, '\n');
        if(q)
            *q = '\0';

        if(copy != p)
            memmove(copy, p, strlen(p) + 1);

        return copy; 
    }

    void HTTPClient::parseHeader(void * buffer, size_t size, size_t nmemb, HttpHeadInfo & hInfo) {
        /**
         * refer from curl tool_cb_hdr.c
         */
        const char *str = (char *)buffer;
        const size_t cb = size * nmemb;
        const char *end = (char*)buffer + cb;
        const char * ptKey = "Content-disposition:";

        if( (cb > 20) && 0 == strncasecmp(ptKey, str, strlen(ptKey))) {
            const char *p = str + 20;

            /* look for the 'filename=' parameter
             *        (encoded filenames (*=) are not supported) */
            for(;;) {
                char *filename;
                size_t len;

                while(*p && (p < end) && !isalpha((int)  ((unsigned char)(*p))))
                    p++;
                if(p > end - 9) {
                    hInfo.b_cb_filename = false;
                    break;
                }
                if(memcmp(p, "filename=", 9)) {
                    /* no match, find next parameter */
                    while((p < end) && (*p != ';'))
                        p++;
                    continue;
                }
                p += 9;
                /* this expression below typecasts 'cb' only to avoid
                 *warning: signed and unsigned type in conditional expression
                 **/
                len = (ssize_t)cb - (p - str);
                filename = parse_filename(p, len);
                if(filename) {
                    hInfo.b_cb_filename = true;
                    hInfo.fileName = filename;
                    free(filename);
                    filename = NULL;
                    break;
                }
                else {
                    hInfo.b_cb_filename = false;
                    return;
                }
            }//for 
        }
        else {
            hInfo.b_cb_filename = false;
        }    
    }

    size_t HTTPClient::httpRspHeader(void *buffer, size_t size, size_t nmemb, void *stream) {

        HTTPClient * ptrCli = (HTTPClient *)(stream);
        if (ptrCli != NULL && ptrCli->getSharedPtrofMsgListener()) {
            //get head info
            HttpHeadInfo hInfo;
            ptrCli->parseHeader(buffer, size, nmemb, hInfo);    

            std::shared_ptr<HttpMsgListener> li = ptrCli->getSharedPtrofMsgListener();
            li.get()->onHead(hInfo);        
        }
        else {
            fprintf(stderr, "ptrCli or ptrCli->getSharedPtrofMsgListener is null\n");
        }
        return size*nmemb; 
    }

    size_t HTTPClient::httpRspCallBack(void *buffer, size_t size, size_t nmemb, void *stream) {
        HTTPClient * ptrCli = (HTTPClient *)(stream);
        if (ptrCli != NULL && ptrCli->getSharedPtrofMsgListener()) {
            std::shared_ptr<HttpMsgListener> li = ptrCli->getSharedPtrofMsgListener();
            li.get()->onData(buffer, size*nmemb);        
        }
        else {
            fprintf(stderr, "ptrCli or ptrCli->getSharedPtrofMsgListener is null\n");
        }
        return size*nmemb; 
    }

    void HTTPClient::getHelper(const std::string &url, std::shared_ptr<HttpMsgListener> ptLi) {

        if (!ptLi) {
            printf("HttpMsgListener is null\n");
            return;
        }

        this->mPtMsgListener = ptLi;
        CURL *curl = nullptr;
        CURLcode res;

        std::shared_ptr<HttpMsgListener> li = this->getSharedPtrofMsgListener();

        curl = curl_easy_init();
        if(curl) {
            /*
             * You better replace the URL with one that works!
             */
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            /*for thread-safe*/
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, true);

            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, httpRspHeader);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

            /* Define our callback to get called when there's data to be written */
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpRspCallBack);
            /* Set a pointer to our struct to pass to the callback */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

            /* Switch on full protocol/debug output */
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

            res = curl_easy_perform(curl);

            /* always cleanup */
            curl_easy_cleanup(curl);

            if(CURLE_OK != res) {
                /* we failed */
                fprintf(stderr, "curl told us %d\n", res);
                li.get()->onError(HttpReqErr::HTTP_CURL_EASY_PERFORM_FAIL, "curl_easy_perform fail");        
            }
        }
        else {
            fprintf(stderr, "curl_easy_init return curl==null\n");
            li.get()->onError(HttpReqErr::HTTP_CURL_EASY_INIT_FAIL, "curl_easy_init return curl==null");        
        }

        li.get()->onFinish();
    }  

    void HTTPClient::get(const std::string &url, std::shared_ptr<HttpMsgListener> ptLi) {
        std::thread getThread(std::mem_fn(&HTTPClient::getHelper), this, url, ptLi);
        getThread.detach();
    }

    void HTTPClient::postHelper(const std::string &url, const std::string &reqBody, std::shared_ptr<HttpMsgListener> ptLi) {

        if (!ptLi) {
            printf("HttpMsgListener is null\n");
            return;
        }

        this->mPtMsgListener = ptLi;
        CURL *curl;
        CURLcode res;

        std::shared_ptr<HttpMsgListener> li = this->getSharedPtrofMsgListener();

        /* get a curl handle */
        curl = curl_easy_init();
        if(curl) {
            /* First set the URL that is about to receive our POST. This URL can
               just as well be a https:// URL if that is what should receive the
               data. */
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, httpRspCallBack); 

            /* Set a pointer to our struct to pass to the callback */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

            /*for thread-safe*/
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, true);

            struct curl_slist *plist = curl_slist_append(NULL,   
                    "Content-Type:application/json;charset=UTF-8");  
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);

            /* Now specify the POST data */
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqBody.c_str());

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);

            /* Check for errors */
            if(res != CURLE_OK) {

                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
                li.get()->onError(HttpReqErr::HTTP_CURL_EASY_PERFORM_FAIL, "curl_easy_perform fail");        
            }
            /* always cleanup */
            curl_easy_cleanup(curl);
        }
        else {
            fprintf(stderr, "curl_easy_init return curl==null\n");
            li.get()->onError(HttpReqErr::HTTP_CURL_EASY_INIT_FAIL, "curl_easy_init return curl==null");        
        }

        li.get()->onFinish();
    }

    void HTTPClient::post(const std::string &url, const std::string &reqBody, std::shared_ptr<HttpMsgListener> ptLi) {

        std::thread postThread(std::mem_fn(&HTTPClient::postHelper), this, url, reqBody, ptLi);
        postThread.detach();
    }
}
