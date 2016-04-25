/**
 * @file dev_plugin.h
 * @brief interface of device plugin on gwagent for device party
 * @author europelee, europelee@gmail.com
 * @version 0.0.1
 * @date 2016-04-13
 */

#ifndef DEV_PLUGIN_H
#define DEV_PLUGIN_H
#include <stdint.h>
#define IN
#define OUT
#define INOUT

#define DEVNAME_MAXSIZE 256

struct dev_plugin;

typedef int(* devplug_init_inf)(dev_plugin * pt_plugin);
typedef int(devplug_fin_inf)();

typedef void(* devplugin_proccallback)(int retCode, const char * retStream, int len);
typedef void(* devplugin_proc)(IN const char * inStream, int len, devplugin_proccallback cb);
typedef void(* devplugin_collect)(OUT char ** outStream, OUT int * len);

typedef struct dev_plugin {
    int         dev_pluginversion;
    char        dev_pluginname[DEVNAME_MAXSIZE];
    uint32_t    dev_datacollectTick; //second
    devplugin_proc  proc_func;
    devplugin_collect collect_func;
}dev_plugin;

int devplugin_init(INOUT dev_plugin * pt_plugin);
int devplugin_fin();

#endif
