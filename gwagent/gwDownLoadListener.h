/**
 * @file gwDownLoadListener.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-16
 */

#ifndef _GWDOWNLOAD_LISTENER_H
#define _GWDOWNLOAD_LISTENER_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "easylogging++.h"
#include "httpmsglistener.h"

namespace pigeon {

    class gwDownLoadListener : public pigeon::HttpMsgListener {
        private:
            FILE * fp;
            std::string mFileName;
        public:
            gwDownLoadListener(std::string fileName):fp(NULL), mFileName(fileName) {}
            virtual void onHead(const pigeon::HttpHeadInfo & hInfo) override {
                if (hInfo.b_cb_filename) {
                    mFileName = hInfo.fileName;
                }
            }
            virtual void onData(void * buffer, size_t len) override{
                if (fp==NULL) {
                    fp = fopen(mFileName.c_str(), "wb");
                    if (fp==NULL) {
                        LOG(ERROR)<<std::string(strerror(errno));
                        return;
                    }
                }
                fwrite(buffer, len, 1, fp);
            }

            virtual void onFinish() override {
                LOG(INFO)<<"finish http get request";
                if (fp!=NULL) {
                    fflush(fp);
                    fclose(fp);
                    fp = NULL;
                }    
            }

            virtual void onError(HttpReqErr errCode, const std::string & errInfo) override {
            }
    };
}
#endif
