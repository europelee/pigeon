/**
 * @file GatewayPropSon.h
 * @brief parse json, check if valid with json-schema
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-03-04
 */
#ifndef _GATEWAYPROPSON_H
#define _GATEWAYPROPSON_H

#include<string>

namespace pigeon {
    class GatewayPropSon {
        public:
            GatewayPropSon();
            ~GatewayPropSon();
            std::string getPropJson(); 
            bool parsePropJson(const char * propJson);
            void setOnLine(bool bOn);
            int getOnLine();
            const char * getOnLineTag();
        private:
            /** Non-copyable */
            GatewayPropSon(const GatewayPropSon&) =delete;
            GatewayPropSon& operator=(const GatewayPropSon&) =delete;
            GatewayPropSon(GatewayPropSon &&) =delete;
            GatewayPropSon& operator=(GatewayPropSon &&) =delete;
        private:
            static const char * online_proptag;
        private:
            int online;
    };
}
#endif
