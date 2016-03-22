/**
 * @file httpheadinfo.h
 * @brief represent for http header
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-16
 */

#ifndef _HTTP_HEADINFO_H
#define _HTTP_HEADINFO_H

#include<string>
namespace pigeon {
    typedef struct HttpHeadInfo {
        public:   
            bool        b_cb_filename;
            std::string fileName;
    }HttpHeadInfo;
}
#endif
