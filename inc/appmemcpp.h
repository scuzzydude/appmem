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
		~CAppMem();
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
	    ~CAppMemFlat();
		AM_RETURN write32(UINT32 offset, UINT32 val);
		AM_RETURN read32(UINT32 offset, UINT32 *pVal);
		UINT32 count(void);

		UINT32& operator[] (const unsigned int idx); // x = flat[idx]
		//UINT32& operator[] (unsigned int idx) const;

	private:
		virtual AM_RETURN configCaps(AM_MEM_CAP_T *pCap);
		UINT32 mem_size;
		UINT32 address_size;
		UINT32 elements;
		UINT32 scratch_pad;

};


class CAppMemStaticArray: public CAppMem
{
	public:
		CAppMemStaticArray(char *am_name, UINT32 elemCount, UINT32 dataSize, UINT32 indexSize);
	    ~CAppMemStaticArray();
		AM_RETURN insert(UINT32 index, void *pVal);
		AM_RETURN get(UINT32 index, void *pVal);
		UINT32 count(void);

		UINT32& operator[] (const unsigned int idx); // x = flat[idx]

	private:
		virtual AM_RETURN configCaps(AM_MEM_CAP_T *pCap);
		UINT32 elements;
		UINT32 mem_size;
		UINT32 index_size;
		UINT32 data_size;
		UINT32 scratch_pad;

};






#endif
