/**
 * @file MPEndListener.h
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-02-17
 */
#ifndef _MPEND_LISTENER_H
#define _MPEND_LISTENER_H

#include<string>
class MPEndListener {
    public:
        virtual void onData(const std::string & topic, const std::string & content)=0;
};
#endif
