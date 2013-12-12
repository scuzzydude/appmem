/*****************************************************************************************
TBD
https://github.com/scuzzydude/appmem
********************************************************************************************/
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_list.h"
#include "am_targ.h"


AM_RETURN am_list_add_fix(AM_MEM_FUNCTION_T *pFunc, void *p1)
{
	AMLIB_LIST *pList;
	AMLIB_LIST_ITEM *pItem;

	AM_ASSERT(pFunc);
	AM_ASSERT(p1);
	pList = (AMLIB_LIST *)pFunc->pVdF;


	pItem = (AMLIB_LIST_ITEM *)AM_MALLOC(sizeof(AMLIB_LIST_ITEM) + pList->data_size);
	pItem->data_size = pList->data_size;
	pItem->data = (void *)&pItem[1];
	memcpy(pItem->data, p1, pItem->data_size);

	if(AM_LIST_SLL == pList->list_type)
	{
		LL_PREPEND(pList->head, pItem);
	}

	return AM_RET_GOOD;
}



AM_RETURN am_create_list_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
	AM_RETURN error = AM_RET_GOOD;
	UINT8 bFixedData = TRUE;
	AMLIB_LIST *pVdF = NULL;


	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


	AM_DEBUGPRINT( "am_create_list_device: maxSize=%lld\n", pCap->maxSize );

	pVdF = (AMLIB_LIST *)AM_MALLOC(sizeof(AMLIB_LIST));

	if(pVdF && pCap && (AM_TYPE_LIST == pCap->amType))
	{
		pVdF->list_type = pCap->subType;
		pVdF->elem_count = 0;
		pVdF->head = NULL;
		pVdF->data_size = pCap->typeSpecific[TS_LIST_DATA_MAX_SIZE];
		pVdF->data_type = pCap->typeSpecific[TS_LIST_DATA_TYPE];


		pFunc->crResp.data_size = pVdF->data_size;


		pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = (am_fn_op_only)am_targ_release;
		pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;

		pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = (am_fn_op_only)am_targ_open;
		pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

		pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = (am_fn_op_only)am_targ_close;
		pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

		if(pVdF->data_type & TS_LIST_DATA_FIXED)
		{
			//				pFunc->pfnOps[AM_OP_READ_ALIGN].align  = am_assca_read32_align;
			//				pFunc->crResp.ops[AM_OP_READ_ALIGN] = (AM_KPACK << 16) | AM_OP_READ_ALIGN;

			pFunc->pfnOps[AM_OP_ADD_ALIGN].config  = am_list_add_fix;
			pFunc->crResp.ops[AM_OP_ADD_ALIGN] = (AM_KPACK << 16) | AM_OP_WRITE_ALIGN;

			//				pFunc->crResp.acOps[ACOP_WRITE] = pFunc->crResp.ops[AM_OP_WRITE_ALIGN];
			//				pFunc->crResp.acOps[ACOP_READ]  = pFunc->crResp.ops[AM_OP_READ_ALIGN];


		}


		pFunc->pVdF = (AM_FUNC_DATA_U *)pVdF;
		sprintf(pFunc->crResp.am_name, "am_list_%04x", pFunc->handle);
		AM_DEBUGPRINT("CREATE LIST = AM_NAME=%s\n", pFunc->crResp.am_name);		
		pFunc->crResp.devt = pFunc->handle;
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
			TS_LIST_MAX_TS,
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
			TS_LIST_MAX_TS,
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
			TS_LIST_MAX_TS,
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
