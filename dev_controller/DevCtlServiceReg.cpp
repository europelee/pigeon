/**
 * @file DevCtlServiceReg.cpp
 * @brief 
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-01
 */
#include <stdlib.h>
#include <string.h>
#include "DevCtlServiceReg.h"

DevCtlServiceReg::DevCtlServiceReg(const std::string & zkAddrList, const std::string & nodePath, const std::string & nodeValue):mZKAddrList(zkAddrList), mNodePath(nodePath), mNodeValue(nodeValue) , mNodeRealPath(""), mZKHdl(NULL){

}

DevCtlServiceReg::~DevCtlServiceReg() {
    if (NULL != mZKHdl) {
        zookeeper_close(mZKHdl);
        mZKHdl = NULL;
    }
}

int DevCtlServiceReg::RegService() {

    int timeout = 30000;
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
    
    if (NULL == mZKHdl) {
        mZKHdl = zookeeper_init(mZKAddrList.c_str(), NULL, timeout, 0, (char *)"Config dev_controller", 0);
    }

    if (mZKHdl ==NULL)  
    {  
        fprintf(stderr, "Error when connecting to zookeeper servers...\n");  
        return -1;
    }

    int ret = mkdirp(mZKHdl, mNodePath.c_str(), mNodePath.c_str());
    if (ret != 0) {
        printf("mkdirp fail\n");
        return -1;
    }

    ret = zoo_set(mZKHdl, mNodeRealPath.c_str(),  mNodeValue.c_str(), mNodeValue.size(), -1);
    if(ret != ZOK){
        fprintf(stderr,"failed to set the data of path %s!\n", mNodeRealPath.c_str());
        return -1;
    }

    return 0;
}

int DevCtlServiceReg::createNode(zhandle_t *zkhandle, const char * path, int flags) {

    char path_buffer[512];  
    int bufferlen=sizeof(path_buffer); 
    int ret = zoo_exists(zkhandle, path,0,NULL);
    if(ret != ZOK){
        ret = zoo_create(zkhandle,path, NULL,0,&ZOO_OPEN_ACL_UNSAFE, flags, path_buffer,bufferlen);
        printf("path_buffer:%s\n", path_buffer);
        mNodeRealPath = path_buffer;
        if(ret != ZOK){
            fprintf(stderr, "Error when create path :%s retcode:%d\n", path, ret);  
            return -1;  
        }

        return 0;
    }
    else {
        return 0;
    }
}

int DevCtlServiceReg::mkdirp(zhandle_t *zkhandle, const char * path, const char * lpath) {
    int c = '/';
    const char * p = strchr(path, c);
    if (p == NULL) {
        return -1;
    }
    else {
        const char * n = p+1;
        if (n == NULL) {
            return -1;
        }
        const char * p2 = strchr(n, '/');
        if (p2 == NULL) {
            //the leaf node
            return createNode(zkhandle, lpath, ZOO_EPHEMERAL|ZOO_SEQUENCE);
        }
        else {
            int len = p2 - lpath;
            if (p2 - n <= 0) {
                return -1;
            }
            else {
                char * tmp = (char *)malloc(len+1);
                strncpy(tmp, lpath, len);
                tmp[len] = 0;
                int ret = createNode(zkhandle, tmp, 0);
                free(tmp);
                tmp = NULL;
                if (ret != 0) {
                    return -1;
                }

                ret = mkdirp(zkhandle, p2, lpath);
                return ret;
            }
        }

    }
}
