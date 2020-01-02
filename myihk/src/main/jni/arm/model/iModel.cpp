#include <unistd.h>
#include <dlfcn.h>
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
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
void default_onPreCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
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
       "r0=0x%lx, r1=0x%lx, r2=0x%lx, r3=0x%lx, r4=0x%lx, r5=0x%lx, r6=0x%lx, r7=0x%lx, r8=0x%lx, r9=0x%lx, r10=0x%lx, r11=0x%lx, r12=0x%lx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/13=0x%lx, lr=0x%lx, cur_pc=0x%lx, cpsr=0x%lx, "
       "arg4=0x%lx, arg5=0x%lx, arg4=0x%lx, arg5=0x%lx;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12],
       regs, ((char*)regs + 0x44), regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16],
       regs->uregs[17], regs->uregs[18], SP(0), SP(1)
    );

    if (pInfo) {
        LE("onPreCallBack: HK_INFO=%p", pInfo);
        if ((pInfo->pBeHookAddr == open || pInfo->pBeHookAddr == dlsym(RTLD_DEFAULT, "__open")) && regs->uregs[0]) {
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
       "r0=0x%lx, r1=0x%lx, r2=0x%lx, r3=0x%lx, r4=0x%lx, r5=0x%lx, r6=0x%lx, r7=0x%lx, r8=0x%lx, r9=0x%lx, r10=0x%lx, r11=0x%lx, r12=0x%lx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/13=0x%lx, lr=0x%lx, cur_pc=0x%lx, cpsr=0x%lx, "
       "arg4=0x%lx, arg5=0x%lx, arg4=0x%lx, arg5=0x%lx;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12],
       regs, ((char*)regs + 0x44), regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16],
       regs->uregs[17], regs->uregs[18], SP(0), SP(1)
    );

    if (pInfo) {
        LE("onCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            LE("onCallBack: open ret: %d", regs->uregs[0]);
        }
    }

}



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

    //DEMO只很对ARM指令进行演示，更通用这里需要判断区分THUMB等指令
    if(TEST_BIT0((uint32_t)pInfo->pBeHookAddr)){ //thumb mode
        LOGI("HookThumb Start.");
        if(!HookThumb(pInfo))
        {
            LOGI("HookThumb fail.");
            remove(pInfo);
            delete pInfo;
            return status;
        }
    } else if(!HookArm(pInfo))
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

    pInfo->onPreCallBack = onCallBack;
    pInfo->methodName = methodName;
    pInfo->onCallBack = (void (*)(my_pt_regs *, STR_HK_INFO *))(0xffffffff);
    pInfo->pHkFunAddr = NULL;

    if(TEST_BIT0((uint32_t)pInfo->pBeHookAddr)){ //thumb mode
        LOGI("HookThumb Start.");
        if(!HookThumb(pInfo))
        {
            LOGI("HookThumb fail.");
            remove(pInfo);
            delete pInfo;
            return status;
        }
    } else if(!HookArm(pInfo))
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

    const void* caller_addr = __builtin_return_address(0);
    LE("caller_addr=%p", caller_addr);
    LE("caller_addr_1=%p", __builtin_return_address(1));

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

    if(TEST_BIT0((uint32_t)pInfo->pBeHookAddr)){ //thumb mode
        LOGI("HookThumb Start.");
        if(!HookThumb(pInfo))
        {
            LOGI("HookThumb fail.");
            remove(pInfo);
            delete pInfo;
            return info;
        }
    } else if(!HookArm(pInfo))
    {
        LE("HookArm fail.");
        remove(pInfo);
        delete pInfo;
        return info;
    }

    info.info = pInfo;//(pInfo->pOriFuncAddr);
    info.status = success;

    caller_addr = __builtin_return_address(0);
    LE("caller_addr=%p", caller_addr);
    LE("caller_addr_1=%p", __builtin_return_address(1));

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
