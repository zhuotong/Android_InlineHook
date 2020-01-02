#include "mhk.h"
#include "../fixPCOpcode.h"


extern unsigned long j_dump_start;
extern unsigned long j_hk_info;
extern unsigned long j_dump_end;

//extern unsigned long _dump_start;
//extern unsigned long _hk_info;
//extern unsigned long _dump_end;



/**
 * 利用dump_just_ret.s中的shellcode构造桩，跳转到pInfo->onPreCallBack函数
 * @param  pInfo inlinehook信息
 * @return               inlinehook桩是否构造成功
 */
bool BuildStub_dump_just_ret(HK_INFO* pInfo)
{
    bool bRet = false;

    LE("BuildStub_dump_just_ret");
    while(1)
    {
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }


        void *p_shellcode_start_s = &j_dump_start;
        void *p_shellcode_end_s = &j_dump_end;
        void *p_hk_info = &j_hk_info;

        size_t sShellCodeLength = p_shellcode_end_s - p_shellcode_start_s;
        pInfo->shellCodeLength = sShellCodeLength;

        //malloc一段新的stub代码
        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewShellCode = NULL;// = malloc(sShellCodeLength);
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

        pInfo->hk_infoAddr = pNewShellCode + (p_hk_info - p_shellcode_start_s);
        *(pInfo->hk_infoAddr) = pInfo;

        //填充shellcode地址到hookinfo中，用于构造hook点位置的跳转指令
        pInfo->pStubShellCodeAddr = pNewShellCode;



        bRet = true;
        break;
    }

    return bRet;
}

