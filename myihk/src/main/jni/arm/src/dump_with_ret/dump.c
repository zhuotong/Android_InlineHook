#include "mhk.h"
#include "../fixPCOpcode.h"


extern unsigned long r_dump_start;
extern unsigned long r_hk_info;
extern unsigned long r_dump_end;

//extern unsigned long _dump_start;
//extern unsigned long _hk_info;
//extern unsigned long _dump_end;


#include "lr.h"

/**
 * 利用dump_with_ret.s中的shellcode构造桩，跳转到pInfo->onPreCallBack函数，之后回调老函数，最好执行pInfo->allBack
 * @param  pInfo inlinehook信息
 * @return               inlinehook桩是否构造成功
 */
bool BuildStub_dump_with_ret(HK_INFO* pInfo)
{
    bool bRet = false;

    LE("BuildStub_dump_with_ret");
    while(1)
    {
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }


        void *p_shellcode_start_s = &r_dump_start;
        void *p_shellcode_end_s = &r_dump_end;
        void *p_hk_info = &r_hk_info;

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


        pInfo->hk_infoAddr = pNewShellCode + (p_hk_info - p_shellcode_start_s);
        *(pInfo->hk_infoAddr) = pInfo;

        //填充shellcode地址到hookinfo中，用于构造hook点位置的跳转指令
        pInfo->pStubShellCodeAddr = pNewShellCode;

//        pInfo->pre_callback = pre_callback;
//        pInfo->aft_callback = aft_callback;

        pInfo->pre_callback = d_pre_callback;
        pInfo->aft_callback = d_aft_callback;
        pInfo->ppOriFuncAddr = NULL;

        bRet = true;
        break;
    }

    return bRet;
}

