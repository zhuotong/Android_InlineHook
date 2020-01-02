#include "mhk.h"
#include "../fixPCOpcode.h"


extern unsigned long replace_start;
extern unsigned long p_hk_info;
extern unsigned long replace_end;

//extern unsigned long _dump_start;
//extern unsigned long _hk_info;
//extern unsigned long _dump_end;


#include "lr.h"

/**
 * 利用replace.s中的shellcode构造桩，跳转到pInfo->onPreCallBack函数，之后回调hook函数，最后执行pInfo->allBack
 * @param  pInfo inlinehook信息
 * @return               inlinehook桩是否构造成功
 */
bool BuildStub_replace(HK_INFO* pInfo)
{
    bool bRet = false;

    LE("BuildStub_replace");
    while(1)
    {
        if(pInfo == NULL)
        {
            LOGI("HK_INFO is null");
            break;
        }


        void *p_shellcode_start_s = &replace_start;
        void *p_shellcode_end_s = &replace_end;
        void *t_hk_info = &p_hk_info;

        size_t sShellCodeLength = p_shellcode_end_s - p_shellcode_start_s;
        pInfo->shellCodeLength = sShellCodeLength;
        LE("sShellCodeLength=%ld", sShellCodeLength);
        //malloc一段新的stub代码
        //signal 11 (SIGSEGV), code 2 (SEGV_ACCERR)
        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewShellCode = NULL;// = malloc(sShellCodeLength);
//        void *pNewShellCode = malloc(sShellCodeLength);
        int code = posix_memalign(&pNewShellCode, pagesize, pagesize);
        LE("pNewShellCode=%p", pNewShellCode);
        if(code || pNewShellCode == NULL)
        {
            LOGI("shell code malloc fail.");
            break;
        }
        memcpy(pNewShellCode, p_shellcode_start_s, sShellCodeLength);
        //更改stub代码页属性，改成可读可写可执行
        if(ChangePageProperty(pNewShellCode, sShellCodeLength) == false)
        {
            LOGI("change shell code page property fail.");
            break;
        }

        //设置跳转到外部stub函数去
        /*LOGI("_hookstub_function_addr_s : %lx",p_hookstub_function_addr_s);
        void **ppHookStubFunctionAddr = pNewShellCode + (p_hookstub_function_addr_s - p_shellcode_start_s);
        *ppHookStubFunctionAddr = pInfo->onCallBack;
        LOGI("ppHookStubFunctionAddr : %lx",ppHookStubFunctionAddr);
        LOGI("*ppHookStubFunctionAddr : %lx",*ppHookStubFunctionAddr);

        //备份外部stub函数运行完后跳转的函数地址指针，用于填充老函数的新地址
        pInfo->ppOriFuncAddr  = pNewShellCode + (p_old_function_addr_s - p_shellcode_start_s);*/

        pInfo->hk_infoAddr = pNewShellCode + (t_hk_info - p_shellcode_start_s);
        *(pInfo->hk_infoAddr) = pInfo;

        //填充shellcode地址到hookinfo中，用于构造hook点位置的跳转指令
        pInfo->pStubShellCodeAddr = pNewShellCode;

//        pInfo->pre_callback = pre_callback;
        pInfo->pre_callback = r_pre_callback;
//        pInfo->aft_callback = aft_callback;
        pInfo->aft_callback = r_aft_callback;


        bRet = true;
        break;
    }

    return bRet;
}

