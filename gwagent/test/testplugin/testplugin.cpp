/**
 * @file testplugin.cpp
 * @brief for testing device plugin
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "dev_plugin.h"
static const char * testPlugName = "testDevPlug";
static const int testPlugVersion = 1;
static int dataCollectTick = 10;
static const char * testCollectData0 = "[{\"devid\":\"12adf\", \"devname\":\"testp\", \"prop\":{\"ddddffff\":\"dg\"}}]";
static const char * testInvalidData = "invalid data";
static const char * testCollectData1 = "{\"devid\":\"12adf1\", \"devname\":\"testp1\", \"prop\":\"{\\\"value\\\":212}\"}";
static const char * testCollectData2 = "[{\"devid\":\"12adf1\", \"devname\":\"testp1\", \"prop\":\"{\\\"value\\\":212}\"}]";
static int iIndex = 0;
static const char * pIdx = NULL;

static void testplug_proc(const char * inStream, int len, devplugin_proccallback cb) {
    printf("%s %s called\n", testPlugName, __func__);
}

static void testplug_collect(char ** outStream, int * len) {
    if (outStream == NULL || len == NULL) {
        printf("outStream == NULL || len == NULL\n");
        return;
    }
    
    switch (iIndex) {
        case 0:
            pIdx = testCollectData0;
            break;
        case 1:
            pIdx = testInvalidData;
            break;
        case 2:
            pIdx = testCollectData1;
            break;
        case 3:
            pIdx = testCollectData2;
            break;
        default:
            pIdx = testCollectData0;
            break;
    }

    *len = strlen(pIdx);
    *outStream = (char *)malloc(*len+1);
    strcpy(*outStream, pIdx);
    int tmp = (++iIndex)%4;
    iIndex = tmp;
}

int devplugin_init(INOUT dev_plugin * pt_plugin) {
    int iRet = -1;
    if (NULL == pt_plugin) {
        iRet = -1;
        return iRet;
    }    
    pt_plugin->dev_pluginversion = testPlugVersion;
    strcpy(pt_plugin->dev_pluginname, testPlugName);
    pt_plugin->dev_datacollectTick = dataCollectTick;
    pt_plugin->proc_func = testplug_proc;
    pt_plugin->collect_func = testplug_collect;    
    iRet = 0;
    return iRet;
}

int devplugin_fin() {
    return 0;
}

#ifdef __cplusplus
}
#endif
