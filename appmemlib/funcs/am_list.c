/*****************************************************************************************
TBD
https://github.com/scuzzydude/appmem
********************************************************************************************/
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_list.h"
#include "am_targ.h"




AM_RETURN am_create_list_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
	AM_RETURN error = AM_RET_GOOD;
#if 0
	UINT8 bFixedKey = TRUE;
	UINT8 bFixedData = TRUE;
	AM_FUNC_DATA_U *pVdF = NULL;
	UINT32 key_size;
	UINT32 data_size;
	UINT16 flags = 0;
	AMLIB_ASSCA *pAA = (AMLIB_ASSCA *)NULL;

	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


	AM_DEBUGPRINT( "am_create_assca_device: maxSize=%lld\n", pCap->maxSize );

	pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));

	if(pVdF && pCap && (AM_TYPE_ASSOC_ARRAY == pCap->amType))
	{

		key_size = pCap->typeSpecific[TS_ASSCA_KEY_MAX_SIZE];
		data_size =	pCap->typeSpecific[TS_ASSCA_DATA_MAX_SIZE];

		if(pCap->typeSpecific[TS_ASSCA_KEY_TYPE] & TS_ASSCA_KEY_FIXED_WIDTH)
		{
			bFixedKey = TRUE;
		}
		if(pCap->typeSpecific[TS_ASSCA_DATA_TYPE] & TS_ASSCA_DATA_FIXED_WIDTH)
		{
			bFixedData = TRUE;
		}


		pAA = amlib_assca_init(key_size, data_size, bFixedKey, bFixedData, flags);

		if(NULL != pAA)
		{

			pVdF->assca.size = pCap->maxSize;
			pVdF->assca.cur_count = 0;
			pVdF->assca.data = (void *)pAA;
            pAA->fd = pVdF;
            
			AM_DEBUGPRINT( "am_create_assca_device: pAA=%p\n", pAA);

			pFunc->crResp.data_size = data_size;
			pFunc->crResp.idx_size = key_size;


			pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = (am_fn_op_only)am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;

			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = (am_fn_op_only)am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = (am_fn_op_only)am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;



			if((TRUE == bFixedKey) && (TRUE == bFixedData))
			{

				pFunc->pfnOps[AM_OP_READ_ALIGN].align  = am_assca_read32_align;
				pFunc->crResp.ops[AM_OP_READ_ALIGN] = (AM_KPACK << 16) | AM_OP_READ_ALIGN;

				pFunc->pfnOps[AM_OP_WRITE_ALIGN].align  = am_assca_write32_align;
				pFunc->crResp.ops[AM_OP_WRITE_ALIGN] = (AM_KPACK << 16) | AM_OP_WRITE_ALIGN;


				pFunc->crResp.acOps[ACOP_WRITE] = pFunc->crResp.ops[AM_OP_WRITE_ALIGN];
				pFunc->crResp.acOps[ACOP_READ]  = pFunc->crResp.ops[AM_OP_READ_ALIGN];

				/* To Make Sure that data segment is 32 bit aligned */
				pFunc->crResp.pack_DataOffset = pFunc->crResp.idx_size / sizeof(UINT32);


				if( pFunc->crResp.idx_size % sizeof(UINT32))
				{
					pFunc->crResp.pack_DataOffset++;    
				}

				AM_DEBUGPRINT("PACKET DATA IDX SIZE=%d\n", pFunc->crResp.idx_size);
				AM_DEBUGPRINT("PACKET DATA DATA SIZE=%d\n", pFunc->crResp.data_size);
				AM_DEBUGPRINT("PACKET DATA DATA OFFSET DWORDS=%d\n", pFunc->crResp.pack_DataOffset);


				pFunc->crResp.wr_pack_qword_size = pFunc->crResp.pack_DataOffset;

				pFunc->crResp.wr_pack_qword_size += (pFunc->crResp.data_size / sizeof(UINT32));

				if(pFunc->crResp.data_size % sizeof(UINT32))
				{
					pFunc->crResp.wr_pack_qword_size++;
				}

				pFunc->crResp.wr_pack_qword_size++; /* Always need one for opcode */   

				AM_DEBUGPRINT("WR PACKET SIZE DWORDS=%d\n", pFunc->crResp.wr_pack_qword_size);

				if(pFunc->crResp.wr_pack_qword_size % 2)
				{
					pFunc->crResp.wr_pack_qword_size = 1 + (pFunc->crResp.wr_pack_qword_size / 2);
				}
				else
				{
					pFunc->crResp.wr_pack_qword_size = (pFunc->crResp.wr_pack_qword_size / 2);
				}


				AM_DEBUGPRINT("WR PACKET SIZE QWORDS=%d\n", pFunc->crResp.wr_pack_qword_size);

				pFunc->crResp.rd_pack_qword_size = pFunc->crResp.pack_DataOffset;                    

				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 

				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 
				pFunc->crResp.rd_pack_qword_size++; //read user data pointer - two dwords 


				AM_DEBUGPRINT("RD PACKET SIZE DWORDS=%d\n", pFunc->crResp.rd_pack_qword_size);

				if(pFunc->crResp.rd_pack_qword_size % 2)
				{

					pFunc->crResp.rd_pack_qword_size = 1 + (pFunc->crResp.rd_pack_qword_size / 2);
				}
				else
				{
					pFunc->crResp.rd_pack_qword_size = (pFunc->crResp.rd_pack_qword_size / 2);

				}

				AM_DEBUGPRINT("RD PACKET SIZE QWORDS=%d\n", pFunc->crResp.rd_pack_qword_size);
			//	AM_DEBUGPRINT("WT PACKET SIZE BYTES=%d\n", pDevice->wr_pack_size);
			//	AM_DEBUGPRINT("RD PACKET SIZE BYTES=%d\n", pDevice->rd_pack_size);


			}

			pFunc->pVdF = pVdF;
			sprintf(pFunc->crResp.am_name, "am_assca_%04x", pFunc->handle);
			AM_DEBUGPRINT("CREATE ASSCA = AM_NAME=%s\n", pFunc->crResp.am_name);		
			pFunc->crResp.devt = pFunc->handle;
		}
		else
		{
			error = AM_RET_ALLOC_ERR;
		}
	}
	else
	{
		error = AM_RET_ALLOC_ERR;
	}

	if(AM_RET_GOOD != error)
	{
		if(NULL != pVdF)
		{
			AM_FREE(pVdF);
		}
	}
#endif

	return error;
}


//*************************************************************************************************
//**   Register this Device Function 
//*************************************************************************************************

static AM_MEM_CAP_T list_caps =
{
	AM_TYPE_LIST,
	(1024 * 1024),
	AM_LIST_SUBTYPES,                   /* subType */
	7,
    0,
	{
		1024 * 1024,      /* Max - Data Size Bytes */
		TS_LIST_DATA_FIXED | TS_LIST_DATA_VAR
	}
};

AM_CAP_DETAILS list_cap_details[AM_LIST_SUBTYPES] = 
{
 //Descriptors are 64 bytes 
	
	//             1         2         3         4         5         6
	//   01234567890123456789012345678901234567890123456789012345678901234
	{
		AM_TYPE_LIST,
		"Linked List",
		AM_LIST_SLL,
		"Singly Linked List - ut-list",
		TS_ASSCA_MAX_TS,
		{
		"Max Data Size",
		"Data Type : 1 = Fixed Width, 2 = Variable Width"
		}
	},
	{
		AM_TYPE_LIST,
		"Linked List",
		AM_LIST_DLL,
		"Doubly Linked List - ut-list",
		TS_ASSCA_MAX_TS,
		{
		"Max Data Size",
		"Data Type : 1 = Fixed Width, 2 = Variable Width"
		}
	},
	{
		AM_TYPE_LIST,
		"Linked List",
		AM_LIST_CLL,
		"circular Linked List - ut-list",
		TS_ASSCA_MAX_TS,
		{
		"Max Data Size",
		"Data Type : 1 = Fixed Width, 2 = Variable Width"
		}
	},

};



static AM_FUNCTION_ENTRY list_function_entry =
{
    AM_TYPE_LIST,
    am_create_list_device,
	&list_caps,
	&list_cap_details[0]
};


INITIALIZER(LIST_init)
{
	am_register_am_function(&list_function_entry);
}
