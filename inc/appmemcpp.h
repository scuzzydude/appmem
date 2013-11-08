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



class CAppMemAsscArray: public CAppMem
{
	public:
		CAppMemAsscArray(char *am_name, UINT32 keySize, UINT32 dataSize, bool bFKey, bool bFData, bool bODupe = false);
	    ~CAppMemAsscArray();
		AM_RETURN insert(void *pKey, void *pVal);
		AM_RETURN get(void *pKey, void *pVal);
		UINT32 count(void);

		UINT32& operator[] (const unsigned int idx); // x = flat[idx]

	private:
		virtual AM_RETURN configCaps(AM_MEM_CAP_T *pCap);
		bool bFixedKey;
		bool bFixedData;
		bool bErrorOnDupe; /* TODO: Returns error (checks key first) on duplicate key insertion, otherwise, we replace key blindly and leak */
		UINT32 key_size;
		UINT32 data_size;


};




#endif
