
#include <map>
#include <pthread.h>
#include <vector>

#include "lr.h"

/*typedef std::vector<unsigned long> LRS;
//static LRS lrs;

struct STR_LR {

};
typedef std::map<const void*, LRS*> LR_MAP;

//typedef std::map<pid_t, LR_MAP*> TID_MAP;
typedef std::map<pid_t, LR_MAP*> TID_MAP;*/

static TID_MAP* tid_map;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

TID_MAP * getTid_map(){
//    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    if (tid_map == NULL) {
        tid_map = new TID_MAP;
    }
    pthread_mutex_unlock(&mutex);
    return tid_map;
}


void saveLR(void* key_fun, unsigned long lr){
    pid_t tid = gettid();
    TID_MAP *map = getTid_map();
    pthread_mutex_lock(&mutex);
    auto it = map->find(tid);
    if (it == map->end()) {
        auto lr_map = new LR_MAP;
        auto ls = new LRS;
        ls->push_back(lr);
        lr_map->insert(std::make_pair(key_fun, ls));
        map->insert(std::make_pair(tid, lr_map));
    } else {
        auto lr_map = it->second;
        auto it_vt = lr_map->find(key_fun);
        if (it_vt == lr_map->end()) {
            auto ls = new LRS;
            ls->push_back(lr);
            lr_map->insert(std::make_pair(key_fun, ls));
        } else {
            std::vector<unsigned long> *vt = it_vt->second;
            vt->push_back(lr);
        }
    }
    pthread_mutex_unlock(&mutex);
}

unsigned long getLR(void* key_fun){
    unsigned long lr = 0;
    pid_t tid = gettid();
    TID_MAP *map = getTid_map();
    pthread_mutex_lock(&mutex);
    auto it = map->find(tid);
    if (it == map->end()) {
        LE("what's happened ? not found tid=%d", tid);//理论上不应该出现
    } else {
        auto lr_map = it->second;
        auto it_vt = lr_map->find(key_fun);
        if (it_vt == lr_map->end()) {
            LE("what's happened ? not found LR for=%p in tid=%d", key_fun, tid);//理论上不应该出现
        } else {
            std::vector<unsigned long> *vt = it_vt->second;
            if (!vt || vt->empty()) {
                LE("what's happened ? null LR for=%p in tid=%d", key_fun, tid);
            } else {
                unsigned long size = vt->size();
                lr = (*vt)[size - 1];
                vt->pop_back();
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    return lr;
}

void pre_callback(struct my_pt_regs *regs, HK_INFO* info){
    LE("lr pre_callback");
    saveLR(info->pBeHookAddr, regs->ARM_lr);
    if (info->onPreCallBack)
        info->onPreCallBack(regs, info);
}

void aft_callback(struct my_pt_regs *regs, HK_INFO* info){
    LE("lr aft_callback");
    unsigned long lr = getLR(info->pBeHookAddr);
    regs->ARM_lr = lr;
    if (info->onCallBack)
        info->onCallBack(regs, info);
}

callback r_pre_callback = pre_callback;
callback r_aft_callback = aft_callback;