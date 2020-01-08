//
// Created by EDZ on 2019/12/19.
//

#include <unistd.h>
#include "iModel.h"

static INFOS* infos;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static INFOS *getINFOS(){
    pthread_mutex_lock(&mutex);
    if (infos == NULL) {
        infos = new INFOS;
    }
    pthread_mutex_unlock(&mutex);
    return infos;
}

static HK_INFO* add(HK_INFO *info){
    INFOS *infos = getINFOS();
    HK_INFO *has = NULL;
    pthread_mutex_lock(&mutex);

    for (auto it = infos->begin(); it < infos->end(); ++it) {
        HK_INFO *pInfo = *it;
        if (pInfo->pBeHookAddr == info->pBeHookAddr) {
            has = pInfo;
            break;
        }
    }
    if (!has) {
        infos->push_back(info);
    }
    pthread_mutex_unlock(&mutex);
    return has;
}

/**
 *  通过被hook函数获取数据结构体
 * @param pBeHookAddr       被hook函数
 * @return                  返回HK_INFO结构体
 */
HK_INFO *isHooked(void* pBeHookAddr){
    INFOS *infos = getINFOS();
    HK_INFO *has = NULL;
    pthread_mutex_lock(&mutex);
    for (auto it = infos->begin(); it < infos->end(); ++it) {
        HK_INFO *pInfo = *it;
        if (pInfo->pBeHookAddr == pBeHookAddr) {
            has = pInfo;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return has;
}

/**
 *  通过hook函数获取数据结构体
 * @param hkFun     hook函数
 * @return          返回HK_INFO结构体
 */
HK_INFO *isHookedByHkFun(void* hkFun){
    INFOS *infos = getINFOS();
    HK_INFO *has = NULL;
    pthread_mutex_lock(&mutex);
    for (auto it = infos->begin(); it < infos->end(); ++it) {
        HK_INFO *pInfo = *it;
        if (pInfo->pHkFunAddr == hkFun) {
            has = pInfo;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return has;
}

void remove(HK_INFO *info){
    INFOS *infos = getINFOS();
    pthread_mutex_lock(&mutex);
    for (auto it = infos->begin(); it < infos->end(); ++it) {
        HK_INFO *pInfo = *it;
        if (pInfo == info) {
            infos->erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * 用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等改变游戏逻辑操作
 *
 * @param regs 寄存器结构，保存寄存器当前hook点的寄存器信息
 */

/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
/*static*/ void default_onPreCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
{
    const char* name = "null";
    if (pInfo) {
        if (pInfo->methodName) {
            name = pInfo->methodName;
        } else {
            char buf[20];
            sprintf(buf, "%p", pInfo->pBeHookAddr);
            name = buf;
        }
    }
//    LE("tid=%d onPreCallBack:%s", gettid(), name);

    LE("tid=%d, onPreCallBack:%s, "
       "x0=0x%llx, x1=0x%llx, x2=0x%llx, x3=0x%llx, x4=0x%llx, x5=0x%llx, x6=0x%llx, x7=0x%llx, x8=0x%llx, x9=0x%llx, x10=0x%llx,"
       " x11=0x%llx, x12=0x%llx, x13=0x%llx, x14=0x%llx, x15=0x%llx, x16=0x%llx, x17=0x%llx, x18=0x%llx, x19=0x%llx, x20=0x%llx, "
       "x21=0x%llx, x22=0x%llx, x23=0x%llx, x24=0x%llx, x25=0x%llx, x26=0x%llx, x27=0x%llx, x28=0x%llx, x29/FP=0x%llx, x30/LR=0x%llx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/31=0x%llx, NZCV/32=0x%llx, x0/pc/33=0x%llx, cur_pc=%llx, arg8=%x, arg9=%x, arg10=%x, arg11=%x, "
       "arg12=%x, arg13=%x;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16], regs->uregs[17],
       regs->uregs[18], regs->uregs[19], regs->uregs[20], regs->uregs[21], regs->uregs[22], regs->uregs[23],
       regs->uregs[24], regs->uregs[25], regs->uregs[26], regs->uregs[27], regs->uregs[28], regs->uregs[29], regs->uregs[30],
       regs, /*((char*)regs + 0x110)*/((char*)regs + 0x310), regs->uregs[31], regs->uregs[32], regs->uregs[33], regs->pc,
       SP(0), SP(1), SP(2), SP(3), SP(4), SP(5)
    );

    LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f, d4=%.15f, d5=%.15f, d6=%.15f, d7=%.15f, "
       "d8=%.15f, d9=%.15f, d10=%.15f, d11=%.15f, d12=%.15f, d13=%.15f, d14=%.15f, d15=%.15f, "
       "d16=%.15f, d17=%.15f, d18=%.15f, d19=%.15f, d20=%.15f, d21=%.15f, d22=%.15f, d23=%.15f, "
       "d24=%.15f, d25=%.15f, d26=%.15f, d27=%.15f, d28=%.15f, d29=%.15f, d30=%.15f, d31=%.15f;",
       DREGS(0), DREGS(1), DREGS(2), DREGS(3), DREGS(4), DREGS(5), DREGS(6), DREGS(7),
       DREGS(8), DREGS(9), DREGS(10), DREGS(11), DREGS(12), DREGS(13), DREGS(14), DREGS(15),
       DREGS(16), DREGS(17), DREGS(18), DREGS(19), DREGS(20), DREGS(21), DREGS(22), DREGS(23),
       DREGS(24), DREGS(25), DREGS(26), DREGS(27), DREGS(28), DREGS(29), DREGS(30), DREGS(31)
    );
    LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f, s4=%.15f, s5=%.15f, s6=%.15f, s7=%.15f, "
       "s8=%.15f, s9=%.15f, s10=%.15f, s11=%.15f, s12=%.15f, s13=%.15f, s14=%.15f, s15=%.15f, "
       "s16=%.15f, s17=%.15f, s18=%.15f, s19=%.15f, s20=%.15f, s21=%.15f, s22=%.15f, s23=%.15f, "
       "s24=%.15f, s25=%.15f, s26=%.15f, s27=%.15f, s28=%.15f, s29=%.15f, s30=%.15f, s31=%.15f;",
       FREGS(0), FREGS(1), FREGS(2), FREGS(3), FREGS(4), FREGS(5), FREGS(6), FREGS(7),
       FREGS(8), FREGS(9), FREGS(10), FREGS(11), FREGS(12), FREGS(13), FREGS(14), FREGS(15),
       FREGS(16), FREGS(17), FREGS(18), FREGS(19), FREGS(20), FREGS(21), FREGS(22), FREGS(23),
       FREGS(24), FREGS(25), FREGS(26), FREGS(27), FREGS(28), FREGS(29), FREGS(30), FREGS(31)
    );
    LE("q0=%.15f, q1=%.15f, q2=%.15f, q3=%.15f, q4=%.15f, q5=%.15f, q6=%.15f, q7=%.15f, "
       "q8=%.15f, q9=%.15f, q10=%.15f, q11=%.15f, q12=%.15f, q13=%.15f, q14=%.15f, q15=%.15f, "
       "q16=%.15f, q17=%.15f, q18=%.15f, q19=%.15f, q20=%.15f, q21=%.15f, q22=%.15f, q23=%.15f, "
       "q24=%.15f, q25=%.15f, q26=%.15f, q27=%.15f, q28=%.15f, q29=%.15f, q30=%.15f, q31=%.15f;",
       QREGS(0), QREGS(1), QREGS(2), QREGS(3), QREGS(4), QREGS(5), QREGS(6), QREGS(7),
       QREGS(8), QREGS(9), QREGS(10), QREGS(11), QREGS(12), QREGS(13), QREGS(14), QREGS(15),
       QREGS(16), QREGS(17), QREGS(18), QREGS(19), QREGS(20), QREGS(21), QREGS(22), QREGS(23),
       QREGS(24), QREGS(25), QREGS(26), QREGS(27), QREGS(28), QREGS(29), QREGS(30), QREGS(31)
    );

    if (pInfo) {
        LE("onPreCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            const char* name = (const char *)(regs->uregs[0]);
            LE("onPreCallBack: open: %s , %o, %o", name, regs->uregs[1], (mode_t)regs->uregs[2]);
        }
    }

}

/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
void default_onCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
{
    const char* name = "null";
    if (pInfo) {
        if (pInfo->methodName) {
            name = pInfo->methodName;
        } else {
            char buf[20];
            sprintf(buf, "%p", pInfo->pBeHookAddr);
            name = buf;
        }
    }
//    LE("tid=%d onPreCallBack:%s", gettid(), name);

    LE("tid=%d, onCallBack:%s, "
       "x0=0x%llx, x1=0x%llx, x2=0x%llx, x3=0x%llx, x4=0x%llx, x5=0x%llx, x6=0x%llx, x7=0x%llx, x8=0x%llx, x9=0x%llx, x10=0x%llx,"
       " x11=0x%llx, x12=0x%llx, x13=0x%llx, x14=0x%llx, x15=0x%llx, x16=0x%llx, x17=0x%llx, x18=0x%llx, x19=0x%llx, x20=0x%llx, "
       "x21=0x%llx, x22=0x%llx, x23=0x%llx, x24=0x%llx, x25=0x%llx, x26=0x%llx, x27=0x%llx, x28=0x%llx, x29/FP=0x%llx, x30/LR=0x%llx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/31=0x%llx, NZCV/32=0x%llx, x0/pc/33=0x%llx, cur_pc=%llx, arg8=%x, arg9=%x, arg10=%x, arg11=%x, "
       "arg12=%x, arg13=%x;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16], regs->uregs[17],
       regs->uregs[18], regs->uregs[19], regs->uregs[20], regs->uregs[21], regs->uregs[22], regs->uregs[23],
       regs->uregs[24], regs->uregs[25], regs->uregs[26], regs->uregs[27], regs->uregs[28], regs->uregs[29], regs->uregs[30],
       regs, /*((char*)regs + 0x110)*/((char*)regs + 0x310), regs->uregs[31], regs->uregs[32], regs->uregs[33], regs->pc,
       SP(0), SP(1), SP(2), SP(3), SP(4), SP(5)
    );

    LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f, d4=%.15f, d5=%.15f, d6=%.15f, d7=%.15f, "
       "d8=%.15f, d9=%.15f, d10=%.15f, d11=%.15f, d12=%.15f, d13=%.15f, d14=%.15f, d15=%.15f, "
       "d16=%.15f, d17=%.15f, d18=%.15f, d19=%.15f, d20=%.15f, d21=%.15f, d22=%.15f, d23=%.15f, "
       "d24=%.15f, d25=%.15f, d26=%.15f, d27=%.15f, d28=%.15f, d29=%.15f, d30=%.15f, d31=%.15f;",
       DREGS(0), DREGS(1), DREGS(2), DREGS(3), DREGS(4), DREGS(5), DREGS(6), DREGS(7),
       DREGS(8), DREGS(9), DREGS(10), DREGS(11), DREGS(12), DREGS(13), DREGS(14), DREGS(15),
       DREGS(16), DREGS(17), DREGS(18), DREGS(19), DREGS(20), DREGS(21), DREGS(22), DREGS(23),
       DREGS(24), DREGS(25), DREGS(26), DREGS(27), DREGS(28), DREGS(29), DREGS(30), DREGS(31)
    );
    LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f, s4=%.15f, s5=%.15f, s6=%.15f, s7=%.15f, "
       "s8=%.15f, s9=%.15f, s10=%.15f, s11=%.15f, s12=%.15f, s13=%.15f, s14=%.15f, s15=%.15f, "
       "s16=%.15f, s17=%.15f, s18=%.15f, s19=%.15f, s20=%.15f, s21=%.15f, s22=%.15f, s23=%.15f, "
       "s24=%.15f, s25=%.15f, s26=%.15f, s27=%.15f, s28=%.15f, s29=%.15f, s30=%.15f, s31=%.15f;",
       FREGS(0), FREGS(1), FREGS(2), FREGS(3), FREGS(4), FREGS(5), FREGS(6), FREGS(7),
       FREGS(8), FREGS(9), FREGS(10), FREGS(11), FREGS(12), FREGS(13), FREGS(14), FREGS(15),
       FREGS(16), FREGS(17), FREGS(18), FREGS(19), FREGS(20), FREGS(21), FREGS(22), FREGS(23),
       FREGS(24), FREGS(25), FREGS(26), FREGS(27), FREGS(28), FREGS(29), FREGS(30), FREGS(31)
    );
    LE("q0=%.15f, q1=%.15f, q2=%.15f, q3=%.15f, q4=%.15f, q5=%.15f, q6=%.15f, q7=%.15f, "
       "q8=%.15f, q9=%.15f, q10=%.15f, q11=%.15f, q12=%.15f, q13=%.15f, q14=%.15f, q15=%.15f, "
       "q16=%.15f, q17=%.15f, q18=%.15f, q19=%.15f, q20=%.15f, q21=%.15f, q22=%.15f, q23=%.15f, "
       "q24=%.15f, q25=%.15f, q26=%.15f, q27=%.15f, q28=%.15f, q29=%.15f, q30=%.15f, q31=%.15f;",
       QREGS(0), QREGS(1), QREGS(2), QREGS(3), QREGS(4), QREGS(5), QREGS(6), QREGS(7),
       QREGS(8), QREGS(9), QREGS(10), QREGS(11), QREGS(12), QREGS(13), QREGS(14), QREGS(15),
       QREGS(16), QREGS(17), QREGS(18), QREGS(19), QREGS(20), QREGS(21), QREGS(22), QREGS(23),
       QREGS(24), QREGS(25), QREGS(26), QREGS(27), QREGS(28), QREGS(29), QREGS(30), QREGS(31)
    );

    if (pInfo) {
        LE("onCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            LE("onCallBack: open ret: %d", regs->uregs[0]);
        }
    }

}

/**
 * 对外inline hook接口，负责管理inline hook信息
 * @param  pHookAddr     要hook的地址
 * @param  onPreCallBack    要插入的回调函数, NULL的情况使用默认的打印寄存器的函数，因为什么都不做为什么hook？
 * @return               inlinehook是否设置成功（已经设置过，重复设置返回false）
 */


/**
 *  真实函数执行前调用onPreCallBack，执行后调用onCallBack，通过onPreCallBack控制参数，通过onCallBack控制返回值
 * @param pBeHookAddr       要hook的地址,必须
 * @param onPreCallBack     要插入的回调函数(读写参数寄存器), 可以为NULL(onCallBack不为空)，当和onCallBack都为NULL的情况使用默认的打印寄存器的函数default_onPreCallBack，因为什么都不做为什么hook？
 * @param onCallBack        要插入的回调函数(读写返回值寄存器)，可以为NULL，如果只关心函数执行后的结果
 * @param methodName        被hook的函数名称，可以为NULL。
 * @return                  success:成功；error:错误；hooked:已被hook；
 */
hk_status dump(void *pBeHookAddr, void (*onPreCallBack)(struct my_pt_regs *, HK_INFO *pInfo), void (*onCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo), const char* methodName)
{
    bool bRet = false;
    LOGI("dump");

    enum hk_status status = error;
    if(pBeHookAddr == NULL)
    {
        return status;
    }

    auto * pInfo = new HK_INFO;
    pInfo->pBeHookAddr = pBeHookAddr;
    HK_INFO *pHkInfo = add(pInfo);
    if (pHkInfo) {
        status = hooked;
        delete(pInfo);
        LE("has hook !");
        return status;
    }

    if (onCallBack) {
        pInfo->onPreCallBack = onPreCallBack;
    } else {
        pInfo->onPreCallBack = onPreCallBack ? onPreCallBack : default_onPreCallBack;
    }
    pInfo->onCallBack = onCallBack;
    pInfo->methodName = methodName;

    pInfo->pHkFunAddr = NULL;

    if(!HookArm(pInfo))
    {
        LE("HookArm fail.");
        remove(pInfo);
        delete(pInfo);
        return status;
    }

    {
        status = success;
    }

    return status;
}

/**
 *  不执行真实函数，直接操作寄存器，之后恢复寄存器返回，理论上也是可以在onCallBack其中执行真实函数的，但是需要自己封装参数，调用后自己解析寄存器
 * @param pBeHookAddr       要hook的地址,必须
 * @param onCallBack        要插入的回调函数(读写参数寄存器)，必须
 * @param methodName        被hook的函数名称，可以为NULL。
 * @return                  success:成功；error:错误；hooked:已被hook；
 */
hk_status dumpRet(void *pBeHookAddr, void (*onCallBack)(struct my_pt_regs *, HK_INFO *pInfo), const char* methodName){
    bool bRet = false;
    LOGI("dumpRet");

    enum hk_status status = error;
    if (pBeHookAddr == NULL || onCallBack == NULL) {
        return status;
    }

    auto * pInfo = new HK_INFO;
    pInfo->pBeHookAddr = pBeHookAddr;
    HK_INFO *pHkInfo = add(pInfo);
    if (pHkInfo) {
        status = hooked;
        delete(pInfo);
        LE("has hook !");
        return status;
    }

    pInfo->onPreCallBack = (void (*)(struct my_pt_regs *, HK_INFO *pInfo))((unsigned long)onCallBack | 0xff00000000000000);//理论上64位地址只用了48位
    pInfo->methodName = methodName;
    pInfo->onCallBack = NULL;
    pInfo->pHkFunAddr = NULL;

    if(!HookArm(pInfo))
    {
        LE("HookArm fail.");
        remove(pInfo);
        delete (pInfo);
        return status;
    }

    {
        status = success;
    }
    return status;
}

/**
 *  针对标准函数，最常用的hook接口。定义一个和被hook函数原型一致的函数接收处理参数，可直接返回或者调用被备份/修复的原函数
 * @param pBeHookAddr       要hook的地址,必须
 * @param pHkFunAddr        和被hook函数原型一致的函数，接收处理参数，可直接返回或者调用被备份/修复的原函数，必须
 * @param onPreCallBack     要插入的回调函数(读写参数寄存器), 可以为NULL
 * @param onCallBack        要插入的回调函数(读写返回值寄存器)，可以为NULL，如果只关心函数执行后的结果
 * @param methodName        被hook的函数名称，可以为NULL
 * @return                  因为既要区分三种状态，还存在返回备份/修复的原函数的情况，使用结构体存储两个字段，参考demo。
 */
struct RetInfo dump_replace(void *pBeHookAddr, void*pHkFunAddr, void (*onPreCallBack)(struct my_pt_regs *, HK_INFO *pInfo), void (*onCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo), const char* methodName)
{
    bool bRet = false;
    LOGI("dump_replace");

    struct RetInfo info;
    info.info = NULL;
    info.status = error;

    if(pBeHookAddr == NULL || pHkFunAddr == NULL)
    {
        return info;
    }

    auto * pInfo = new HK_INFO();
    pInfo->pBeHookAddr = pBeHookAddr;
    HK_INFO *pHkInfo = add(pInfo);
    if (pHkInfo) {
        info.status = hooked;
        delete(pInfo);
        LE("has hook !");
        return info;
    }
    pInfo->pHkFunAddr = pHkFunAddr;
    pInfo->onPreCallBack = onPreCallBack;// ? onPreCallBack : default_onPreCallBack;
    pInfo->onCallBack = onCallBack;
    pInfo->methodName = methodName;

    if(!HookArm(pInfo))
    {
        LE("HookArm fail.");
        remove(pInfo);
        delete pInfo;
        return info;
    }

    info.info = pInfo;//(pInfo->pOriFuncAddr);
    info.status = success;

    return info;
}

/**
 *  获取备份/修复的被hook函数，主要是不清楚结构体字段的用户或者透明指针的情况
 * @param info
 * @return          返回备份/修复的被hook函数
 */
void* getOriFun(HK_INFO* info){
    if (info) {
        return info->pOriFuncAddr;
    }
    return NULL;
}

/**
 *  获取备份/修复的被hook函数的指针，二级指针；可用于自己保存，推荐存在取消hook的情况下调用getOriFunByHkFun函数
 * @param info
 * @return          返回指向存储备份/修复的被hook函数的指针
 */
void** getPoriFun(HK_INFO* info){
    if (info) {
        return info->ppOriFuncAddr;
    }
    return NULL;
}

/**
 *  通过hook函数获取被hook的函数
 * @param hkFun     hook函数
 * @return          返回被hook函数，如果被取消hook或者未被hook返回NULL
 */
void* getOriFunByHkFun(void* hkFun){
    if (hkFun) {
        HK_INFO *pInfo = isHookedByHkFun(hkFun);
        if (pInfo) {
            return pInfo->pOriFuncAddr;
        }
    }
    return NULL;
}


/**
 *  取消hook，释放shellcode/备份的原方法占用的空间并还原原方法
 * @param info      如果成功会释放这个结构体，所以之后这个结构体/指针不能再用
 * @return          取消成功true，否则false
 */
bool unHook(HK_INFO* info){
    if (!info) {
        return false;
    }
    bool ret = unHk(info);
    if (!ret) {
        return false;
    }

    remove(info);
    free(info);

    return true;
}

/**
 *  取消所有的hook
 * @return
 */
bool unHookAll(){
    INFOS *infos = getINFOS();

    pthread_mutex_lock(&mutex);
    for (auto it = infos->begin(); it < infos->end(); ++it) {
        HK_INFO *pInfo = *it;
        bool ret = unHk(pInfo);
        if (!ret) {
            pthread_mutex_unlock(&mutex);
            return false;
        }

        remove(pInfo);
        free(pInfo);
    }
    pthread_mutex_unlock(&mutex);

    return true;
}
