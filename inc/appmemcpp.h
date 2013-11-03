#ifndef _APPMEMCPP_H
#define _APPMEMCPP_H

extern "C" 
{
#include "appmemlib.h"


}

class CAppMem
{
	public:
		CAppMem();
		CAppMem(char *am_name);
		void initBase(char *am_name, UINT32 amType);
		bool ready(void) { return bReady; }
		const char * driverName(void);

	protected:
		AM_MEM_FUNCTION_T amFunc;
		AMLIB_ENTRY_T	amEntry;
		UINT32 cap_count;		
		AM_MEM_CAP_T *pBaseCaps;
		bool bReady;
		AM_FUNC_CALLS_T *pCalls;
		virtual AM_RETURN configCaps(AM_MEM_CAP_T *pCap);



};

class CAppMemFlat: public CAppMem
{
	public:
		CAppMemFlat(char *am_name, UINT32 memSize, UINT8 address_size);
		AM_RETURN write32(UINT32 offset, UINT32 val);
		AM_RETURN read32(UINT32 offset, UINT32 *pVal);

	private:
		virtual AM_RETURN configCaps(AM_MEM_CAP_T *pCap);
		UINT32 mem_size;
		UINT32 address_size;

};








#endif
