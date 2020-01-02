//
// Created by EDZ on 2019/12/19.
//

#ifndef MY_INHK_LR_H
#define MY_INHK_LR_H

#ifdef __cplusplus

#include <map>
#include <pthread.h>
#include <vector>

typedef std::vector<unsigned long> LRS;
//static LRS lrs;

struct STR_LR {

};
typedef std::map<const void*, LRS*> LR_MAP;

//typedef std::map<pid_t, LR_MAP*> TID_MAP;
typedef std::map<pid_t, LR_MAP*> TID_MAP;

static TID_MAP * getTid_map();

static void saveLR(void* key_fun, unsigned long lr);

static unsigned long getLR(void* key_fun);

#endif


#ifdef __cplusplus
extern "C" {
#endif

#include "mhk.h"

//因为会被导出，所以两个静态库中存在两个同名函数，另一个被覆盖了。
//extern void pre_callback(struct my_pt_regs *regs, HK_INFO* info);
static void pre_callback(struct my_pt_regs *regs, HK_INFO* info);

//extern void aft_callback(struct my_pt_regs *regs, HK_INFO* info);
static void aft_callback(struct my_pt_regs *regs, HK_INFO* info);
typedef void (*callback)(struct my_pt_regs *regs, HK_INFO* info);
extern callback d_pre_callback;// = pre_callback;
extern callback d_aft_callback;// = aft_callback;

#ifdef __cplusplus
}
#endif


#endif //MY_INHK_LR_H
