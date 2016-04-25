#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include"zookeeper.h"  
#include"zookeeper_log.h"  
#include "zoo_lock.h"

static char seq[64] = {0};
static volatile int bStop = 0;
static void sigIntHandler(int sig) {
   printf("sig no %d\n", sig);
   bStop = 1;
}


static void free_String_vector(struct String_vector *v) {
    if (v->data) {
        int32_t i;
        for (i=0; i<v->count; i++) {
            free(v->data[i]);
        }
        free(v->data);
        v->data = 0;
    }
}


static char* printNode(struct String_vector *vector) {
    char *ret = NULL;
    if (vector->data) {
        int i = 0;
        for (i = 0; i < vector->count; i++) {
            char* child = vector->data[i];
            printf("print node data %s\n", child);
        }
    }
    return ret;
}

static void childNodeWatch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx) {

    printf("childNodeWatch happened: type[%d], path: %s watcherCtx: %s\n", type, path, (char *)watcherCtx);

    char * childPath = watcherCtx;
    char value[128] = {0};
    int value_len = sizeof(value);
    struct Stat item_stat;
    int ret = zoo_wget(zkhandle, watcherCtx, childNodeWatch, watcherCtx, value, &value_len, &item_stat);
    if (ret) {
        printf("zoo_wget %s error\n", childPath);
    } else {
        printf("zoo_wget %s:%s\n", childPath, value);
    }
    return;
}

static void watchChildValue(zhandle_t * zkhandle, const char * dirPath, struct String_vector * vector) {

    if (vector->data) {
        int i = 0;
        char childPath[128] = {0};
        for (i = 0; i < vector->count; i++) {
            char* child = vector->data[i];
            sprintf(childPath, "%s/%s", dirPath, child);
            char value[128] = {0};
            int value_len = sizeof(value);
            struct Stat item_stat;
            int ret = zoo_wget(zkhandle, childPath, childNodeWatch, childPath, value, &value_len, &item_stat);
            if (ret) {
                printf("zoo_wget %s error\n", childPath);
            } else {
                printf("zoo_wget %s:%s\n", childPath, value);
            }
        }
    }
}

static int createNode(zhandle_t *zkhandle, const char * path, int flags) {

    char path_buffer[512];  
    int bufferlen=sizeof(path_buffer); 
    int ret = zoo_exists(zkhandle, path,0,NULL);
    if(ret != ZOK){
        ret = zoo_create(zkhandle,path, NULL,0,&ZOO_OPEN_ACL_UNSAFE, flags, path_buffer,bufferlen);
        printf("path_buffer:%s\n", path_buffer);
        strcpy(seq, path_buffer);
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

static int mkdirp(zhandle_t *zkhandle, const char * path, const char * lpath) {
    char * p = strchr(path, '/');
    if (p == NULL) {
        return -1;
    }
    else {
        char * n = p+1;
        if (n == NULL) {
            return -1;
        }
        char * p2 = strchr(n, '/');
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
                char * tmp = malloc(len+1);
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
static void zkr_lock_cb(int rc, void* cbdata) {
    printf("lock_cb rc=%d process %s\n", rc, cbdata);

}

static void childWatcherCallBack(zhandle_t* zh, int type, int state,
        const char* path, void* watcherCtx)
{

    printf("child event happened: type[%d], watcherCtx: %s\n", type, (char *)watcherCtx);

    int ret = 0;
    struct String_vector strings;
    struct Stat stat;
    ret = zoo_wget_children2(zh, path, childWatcherCallBack, watcherCtx, &strings, &stat);
    if (ret) {
        fprintf(stderr, "child: zoo_wget_children2 error [%d]\n", ret);
        return;
    }

    if (strings.count == 0) return;

    printNode(&strings);
    watchChildValue(zh, path, &strings);
    free_String_vector(&strings);
    return;
} 
static int watchChildren(zhandle_t *zh, const char *path, watcher_fn watcher, void *watcherCtx) 
{
    int ret = 0;
    struct String_vector strings;
    struct Stat stat;

    ret = zoo_wget_children2(zh, path, watcher, watcherCtx, &strings, &stat);
    if (ret) {
        fprintf(stderr, "zoo_wget_children2 error [%d]\n", ret);
    }
    printf("==========================================\n");
    printNode(&strings);
    free_String_vector(&strings);
    printf("==========================================\n");
    return ret;
}

int main(int argc, char ** argv) {
    if (argc < 4) {
        printf("usage: ./test-zookeeper zksvrIp:zksvrPort nodePath value(like zmqsvrIp:zmqsvrPort) [recipe](eg lock)\n");
        return -1;
    }

    int recipe = -1; //0: test 'lock'
    if (argc > 4) {
        if (0 == strcmp(argv[4], "lock")) {
            recipe = 0;
        }
    }
    
    signal(SIGINT, sigIntHandler);
    signal(SIGTERM, sigIntHandler);
    char dirPath[128] = {0};
    strcpy(dirPath, argv[2]);
    dirPath[127] = 0;
    char * pEnd = strrchr(dirPath, '/');
    *pEnd = 0;    
    int timeout = 30000;
    zoo_set_debug_level(ZOO_LOG_LEVEL_DEBUG);
    zhandle_t* zkhandle = zookeeper_init(argv[1], NULL/*zktest_watcher_g*/, timeout, 0, dirPath, 0);
    if (zkhandle ==NULL)  
    {  
        fprintf(stderr, "Error when connecting to zookeeper servers...\n");  
        return -1;
    }


    zkr_lock_mutex_t mutex;
    if (recipe >= 0) {
        //test recipes
        printf("try lock\n");
        char* path = "/test-lock";
        char  pinfo[32] = {0};
        sprintf(pinfo, "%d", getppid());
        zkr_lock_init_cb(&mutex, zkhandle, path, &ZOO_OPEN_ACL_UNSAFE, &zkr_lock_cb, pinfo);
        zkr_lock_lock(&mutex);
    }
    else { 
    
        int ret = watchChildren(zkhandle, dirPath, childWatcherCallBack, dirPath);
        if (ret) {
            fprintf(stderr, "add_children_watch_on error [%d]\n", ret);
        }
        
        ret = mkdirp(zkhandle, argv[2], argv[2]);
        if (ret != 0) {
            printf("mkdirp fail\n");
            return -1;
        }

        ret = zoo_set(zkhandle,seq,argv[3],strlen(argv[3]),-1);
        if(ret != ZOK){
            fprintf(stderr,"failed to set the data of path %s!\n",argv[2]);
        }
        
    }
    
    while(!bStop) {
        sleep(1);
    }

    if (recipe >= 0) {
        
        printf("process %d: lock ret: id %s ownerid %s\n",getppid(), mutex.id, mutex.ownerid);
        if (0 == strcmp(mutex.id, mutex.ownerid)) {
            zkr_lock_unlock(&mutex);
        }
        
        zkr_lock_destroy(&mutex);
    }

    zookeeper_close(zkhandle);
    return 0;
}
