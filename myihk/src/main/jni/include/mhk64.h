//
// Created by EDZ on 2019/12/18.
//

#ifndef MY_INHK_MHK64_H
#define MY_INHK_MHK64_H

//废弃，因为多个文件包含但是不是单独编译成动态库的话就会出现覆盖的情况


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

extern TID_MAP * getTid_map();

extern void saveLR(void* key_fun, unsigned long lr);

extern unsigned long getLR(void* key_fun);

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void pre_callback(struct my_pt_regs *regs, HK_INFO* info);

extern void aft_callback(struct my_pt_regs *regs, HK_INFO* info);

#ifdef __cplusplus
}
#endif

#endif //MY_INHK_MHK64_H
