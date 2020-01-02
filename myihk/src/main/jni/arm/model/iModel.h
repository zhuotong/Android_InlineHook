//
// Created by EDZ on 2019/12/19.
//

#ifndef MY_INHK_IMODEL_H
#define MY_INHK_IMODEL_H

#include "mhk.h"

#include <jni.h>
#include <fcntl.h>

#ifdef __cplusplus


#include <map>
#include <pthread.h>
#include <vector>

typedef std::vector<HK_INFO*> INFOS;
//typedef std::map<const void*, HK_INFO*> LR_MAP;

#endif

enum hk_status{
    success, hooked, error
};

struct RetInfo {
    enum hk_status status;
    HK_INFO *info;
};

//打印寄存器
/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
void default_onPreCallBack(my_pt_regs *regs, HK_INFO *pInfo);
/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
void default_onCallBack(my_pt_regs *regs, HK_INFO *pInfo);

/**
 * 真实函数执行前调用onPreCallBack，执行后调用onCallBack，通过onPreCallBack控制参数，通过onCallBack控制返回值
 * @param pBeHookAddr       要hook的地址,必须
 * @param onPreCallBack     要插入的回调函数(读写参数寄存器), 可以为NULL(onCallBack不为空)，当和onCallBack都为NULL的情况使用默认的打印寄存器的函数default_onPreCallBack，因为什么都不做为什么hook？
 * @param onCallBack        要插入的回调函数(读写返回值寄存器)，可以为NULL，如果只关心函数执行后的结果
 * @param methodName        被hook的函数名称，可以为NULL。
 * @return                  success:成功；error:错误；hooked:已被hook；
 */
hk_status dump(void *pBeHookAddr, void (*onPreCallBack)(struct my_pt_regs *, HK_INFO *pInfo), void (*onCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo) = NULL, const char* methodName = NULL);

/**
 *  不执行真实函数，直接操作寄存器，之后恢复寄存器返回，理论上也是可以在onCallBack其中执行真实函数的，但是需要自己封装参数，调用后自己解析寄存器
 * @param pBeHookAddr       要hook的地址,必须
 * @param onCallBack        要插入的回调函数(读写参数寄存器)，必须
 * @param methodName        被hook的函数名称，可以为NULL。
 * @return                  success:成功；error:错误；hooked:已被hook；
 */
hk_status dumpRet(void *pBeHookAddr, void (*onCallBack)(struct my_pt_regs *, HK_INFO *pInfo), const char* methodName = NULL);

/**
 *  针对标准函数，最常用的hook接口。定义一个和被hook函数原型一致的函数接收处理参数，可直接返回或者调用被备份/修复的原函数
 * @param pBeHookAddr       要hook的地址,必须
 * @param pHkFunAddr        和被hook函数原型一致的函数，接收处理参数，可直接返回或者调用被备份/修复的原函数，必须
 * @param onPreCallBack     要插入的回调函数(读写参数寄存器), 可以为NULL
 * @param onCallBack        要插入的回调函数(读写返回值寄存器)，可以为NULL，如果只关心函数执行后的结果
 * @param methodName        被hook的函数名称，可以为NULL
 * @return                  因为既要区分三种状态，还存在返回备份/修复的原函数的情况，使用结构体存储两个字段，参考demo。
 */
RetInfo dump_replace(void *pBeHookAddr, void*pHkFunAddr, void (*onPreCallBack)(struct my_pt_regs *, HK_INFO *pInfo) = NULL, void (*onCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo) = NULL, const char* methodName = NULL);

/**
 *  通过被hook函数获取数据结构体
 * @param pBeHookAddr       被hook函数
 * @return                  返回HK_INFO结构体
 */
HK_INFO *isHooked(void* pBeHookAddr);

/**
 *  通过hook函数获取数据结构体
 * @param hkFun     hook函数
 * @return          返回HK_INFO结构体
 */
HK_INFO *isHookedByHkFun(void* hkFun);

/**
 *  获取备份/修复的被hook函数，主要是不清楚结构体字段的用户或者透明指针的情况
 * @param info
 * @return          返回备份/修复的被hook函数
 */
void* getOriFun(HK_INFO* info);

/**
 *  获取备份/修复的被hook函数的指针，二级指针；可用于自己保存，推荐存在取消hook的情况下调用getOriFunByHkFun函数
 * @param info
 * @return          返回指向存储备份/修复的被hook函数的指针
 */
void** getPoriFun(HK_INFO* info);

/**
 *  通过hook函数获取被hook的函数
 * @param hkFun     hook函数
 * @return          返回被hook函数，如果被取消hook或者未被hook返回NULL
 */
void* getOriFunByHkFun(void* hkFun);

/**
 *  取消hook，释放shellcode/备份的原方法占用的空间并还原原方法
 * @param info      如果成功会释放这个结构体，所以之后这个结构体/指针不能再用
 * @return          取消成功true，否则false
 */
bool unHook(HK_INFO* info);

/**
 *  取消所有的hook
 * @return
 */
bool unHookAll();

#endif //MY_INHK_IMODEL_H
