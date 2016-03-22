/**
 * @file HttpReqErr.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-06
 */
#ifndef _HTTPERROR_H
#define _HTTPERROR_H
    enum class HttpReqErr {
        HTTP_CURL_EASY_INIT_FAIL,
        HTTP_CURL_EASY_PERFORM_FAIL
    };

#endif
