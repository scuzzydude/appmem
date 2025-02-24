/*****************************************************************************************

Copyright (c) 2013, Brandon Awbrey
All rights reserved.

https://github.com/scuzzydude/appmem



Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************/

#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"
#include "am_stata.h"
#include "appmem_pack.h"

#ifndef _APPMEMD

AM_RETURN am_pack_op_only(AM_MEM_FUNCTION_T *pFunc, UINT8 op, void *pResp, UINT32 resp_len)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_IDENTIFY);

	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);

	pIop = pFunc->pkAc.get_free_iop(pFunc);
	

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_TYPE_OPCODE_ONLY; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = op;

	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);

	if(AM_RET_GOOD == error)
	{
		if((NULL != pResp) && resp_len)
		{
		
			if((NULL != pIop->pRx) && (pIop->resp_bytes))
			{
				/* TODO: copy buffer, if we ever require it */
				AM_ASSERT(0);
			}
		}
	}

	return error;

}
	
AM_RETURN am_net_close(void * p1)
{
	return am_pack_op_only(p1, AM_OP_CLOSE_FUNC, NULL, 0);
}

AM_RETURN am_net_open(void * p1)
{
	return am_pack_op_only(p1, AM_OP_OPEN_FUNC, NULL, 0);
}


AM_RETURN am_pack_read_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size; 
	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);
	
	tx_size  = sizeof(AM_PACK_WRAPPER_T) + pFunc->crResp.idx_size;

	pIop = pFunc->pkAc.get_free_iop(pFunc);

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_ALIGN; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = AM_OP_READ_ALIGN;

    AM_DEBUGPRINT("PACK_READ align offset = 0x%08x\n", *(UINT32 *)p1);
	
	memcpy(&pIop->pTx->write_al.data_bytes[0], p1, pFunc->crResp.idx_size);

	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);

	if(AM_RET_GOOD == error)
	{
		if((NULL != pIop->pRx) && (pIop->resp_bytes >= (sizeof(AM_PACK_WRAPPER_T) + pFunc->crResp.data_size))) /* tx_size and resp size same on 32bit aligned */
		{
			memcpy(p2, &pIop->pRx->align_resp.resp_bytes[0], pFunc->crResp.data_size);
		}
		else
		{
			error = AM_RET_IO_ERR;
		}
	
	}

	return error;
}

AM_RETURN am_pack_read32_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_READ_ALIGN);
	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);
	
	pIop = pFunc->pkAc.get_free_iop(pFunc);

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_ALIGN; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = AM_OP_READ_ALIGN;
	

    AM_DEBUGPRINT("PACK_READ32 align offset = 0x%08x\n", *(UINT32 *)p1);
    
	*(UINT32 *)&pIop->pTx->write_al.data_bytes[0] = *(UINT32 *)p1;

	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);

	if(AM_RET_GOOD == error)
	{
		if((NULL != pIop->pRx) && (pIop->resp_bytes >= (tx_size))) /* tx_size and resp size same on 32bit aligned */
		{
			*(UINT32 *)p2 = *(UINT32 *) &pIop->pRx->align_resp.resp_bytes[0];
            AM_DEBUGPRINT("PACK_READ32 DATA =%08x \n", *(UINT32 *)&pIop->pRx->align_resp.resp_bytes[0]);
		}
		else
		{
		    error = AM_RET_IO_UNDERRUN;
            AM_DEBUGPRINT("PACK_READ32 error = %d pRx=%p resp_bytes=%d tx_size=%d\n", error, pIop->pRx, pIop->resp_bytes, tx_size);
		}
	
	}
	else
	{
        AM_DEBUGPRINT("PACK_READ32 error = 0x%08x\n", error);
    
	}



	return error;
}


AM_RETURN am_pack_write_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_WRAPPER_T) + pFunc->crResp.data_size + pFunc->crResp.idx_size;


	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);
	
	pIop = pFunc->pkAc.get_free_iop(pFunc);

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_ALIGN; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = AM_OP_WRITE_ALIGN;
	
	memcpy(&pIop->pTx->write_al.data_bytes[0],p1, pFunc->crResp.idx_size);
	memcpy(&pIop->pTx->write_al.data_bytes[pFunc->crResp.idx_size], p2, pFunc->crResp.data_size);
	
	
	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);
	return error;

}



AM_RETURN am_pack_write32_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_WRITE_ALIGN) + 7; /* Basic align has one byte od data */

	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);
	
	pIop = pFunc->pkAc.get_free_iop(pFunc);

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_ALIGN; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = AM_OP_WRITE_ALIGN;
	
	*(UINT32 *)&pIop->pTx->write_al.data_bytes[0] = *(UINT32 *)p1;
	*(UINT32 *)&pIop->pTx->write_al.data_bytes[4] = *(UINT32 *)p2;
	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);
	return error;

}

AM_RETURN am_pack_sort(AM_MEM_FUNCTION_T *pFunc, void * p1, UINT64 l1)
{
	AMLIB_ENTRY_T *pEntry;	
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_WRITE_ALIGN) + ((UINT32)l1) ; /* Basic align has one byte od data */

	AM_ASSERT(pFunc);
	pEntry = pFunc->pEntry;
	AM_ASSERT(pEntry);
	AM_ASSERT(pFunc->pkAc.get_free_iop);
	AM_ASSERT(pFunc->pkAc.send_and_wait);
	

	pIop = pFunc->pkAc.get_free_iop(pFunc);

	pIop->pTx->op.wrap.func_id = (UINT16)pFunc->handle;
	pIop->pTx->op.wrap.packType = AM_PACK_ACTION; 
	pIop->pTx->op.wrap.size = tx_size;
	pIop->pTx->op.wrap.op = AM_OP_SORT;

	memcpy(&pIop->pTx->action.data_in[0], p1,(UINT32)l1); 
	
	error = pFunc->pkAc.send_and_wait(pFunc, pIop, tx_size, 5000);
	return error;

}

AM_RETURN am_set_pack_access(AM_MEM_FUNCTION_T *pFunc)
{


	if((AM_PACK_ALIGN == AM_NET_GET_PACKTYPE(pFunc->crResp.acOps[ACOP_WRITE])) ||
		(AM_KPACK == AM_NET_GET_PACKTYPE(pFunc->crResp.acOps[ACOP_WRITE])))
	{
		if((sizeof(UINT32) == pFunc->crResp.data_size) && (sizeof(UINT32) == pFunc->crResp.idx_size))
		{
			pFunc->fn->write_al = am_pack_write32_align;
		}
		else /* TODO - we should have alignment function for 64/64 or 32/64 and other common cobmos but always fall back to this one */
		{
			pFunc->fn->write_al = am_pack_write_align;
		}
	}
	else ///if(AM_OP_CODE_WRITE_FIX_PACKET == pFunc->crResp.acOps[ACOP_WRITE])
	{
		//printf("write_al = am_kd_fixed_write_packet\n");
		//fn_array->write_al = am_kd_fixed_write_packet;
		//TODO - 
	}

	if((AM_PACK_ALIGN == AM_NET_GET_PACKTYPE(pFunc->crResp.acOps[ACOP_READ])) ||
		(AM_KPACK == AM_NET_GET_PACKTYPE(pFunc->crResp.acOps[ACOP_READ])))
	{

		if((sizeof(UINT32) == pFunc->crResp.data_size) && (sizeof(UINT32) == pFunc->crResp.idx_size))
		{
			pFunc->fn->read_al = am_pack_read32_align;
		}
		else
		{
			pFunc->fn->read_al = am_pack_read_align;
		}

	}
	else
	{
		//TODO: 
	}
	//am_net_print_txrx_buffer(pIop,TRUE);

	return AM_RET_GOOD;

}

#endif


AM_RETURN am_pack_process_cmd(AM_MEM_FUNCTION_T *pFunc, AM_PACK_ALL_U *pRxBuf,AM_PACK_RESP_U *pResp, UINT32 *pTxBytes)
{
    AM_RETURN error = AM_RET_GOOD;
	AM_FN_U *opFn;
	UINT32 op;
    UINT32 tx_bytes;

    AM_ASSERT(pRxBuf);
    AM_ASSERT(pResp);
    AM_ASSERT(pTxBytes);

	/* Copy the wrapper to the response */
	memcpy(&pResp->wrap, &pRxBuf->wrap, sizeof(AM_PACK_WRAPPER_T));

	pResp->wrap.packType |= AM_FUNC_PACK_TYPE_FLAG_RESP;


	if(NULL != pFunc)
	{

		opFn = pFunc->pfnOps;	
		op = pRxBuf->wrap.op;

		AM_ASSERT(opFn);

		if(NULL != opFn[op].raw)
		{
			switch(pRxBuf->wrap.packType)
			{

                case AM_KPACK:   /* Path is different in kernel ioctl mode */
				case AM_PACK_ALIGN:
				{
					if(op & 1)
					{
						/* Read Align */	
						error = opFn[op].align(pFunc, &pRxBuf->write_al.data_bytes[0], &pResp->align_resp.resp_bytes[0]);  
						tx_bytes = sizeof(AM_PACK_WRAPPER_T) + pFunc->crResp.data_size; /* If there is error, it will tx_bytes will be overridden below */

					}
					else
					{
						/* Write Align */
						error = opFn[op].align(pFunc, &pRxBuf->write_al.data_bytes[0], &pRxBuf->write_al.data_bytes[pFunc->crResp.idx_size]);  
						tx_bytes = sizeof(AM_PACK_WRAPPER_T); /* If there is error, it will tx_bytes will be overridden below */
					}
				}
				break;

			
				case AM_PACK_TYPE_OPCODE_ONLY:
				{
					error = opFn[op].op_only(pFunc, pResp, &tx_bytes);			
				}
				break;

				
				case AM_PACK_TYPE_FIVO:
				{
				
					error = opFn[op].fivo(pFunc, 
											&pRxBuf->fivo.data_in[0], 
											pRxBuf->fivo.l1,
											&pResp->align_resp.resp_bytes[0],
											&tx_bytes);

				
				}
				break;

				case AM_PACK_ACTION:
				{
					tx_bytes = sizeof(AM_PACK_WRAPPER_T);
					error = opFn[op].action(pFunc, &pRxBuf->action.data_in[0], (pRxBuf->action.wrap.size - sizeof(AM_PACK_WRAPPER_T)));
				}
				break;



				default:
				error = AM_RET_INVALID_PACK_OP;
				break;

			}
		}
		else
		{
			error = AM_RET_INVALID_OPCODE;
		}


	}
	else
	{
		error = AM_RET_INVALID_FUNC;
	}


	if(AM_RET_GOOD != error)
	{
		pResp->error.wrap.packType |= AM_FUNC_PACK_TYPE_FLAG_ERR;
		pResp->error.status = error;
		pResp->error.wrap.size = sizeof(AM_PACK_RESP_ERR);
		tx_bytes = sizeof(AM_PACK_RESP_ERR);
		AM_DEBUGPRINT("am_pack_process_cmd : pack_type=%04x size = %d\n", pResp->error.wrap.packType, pResp->error.wrap.size); 


	}
	else
	{
		pResp->wrap.size = tx_bytes;
	}


    *pTxBytes = tx_bytes;

    return error;

}


