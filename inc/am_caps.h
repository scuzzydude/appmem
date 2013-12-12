#ifndef _AM_CAPS_H
#define _AM_CAPS_H
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


static AM_MEM_CAP_T virtd_base_cap =
{ 
	AM_TYPE_BASE_APPMEM,
	(1024 * 1024 * 128), /* Total Device Memory */
	5,                   /* subType / function count in basemem */
	2,
	0,
	{
		0, 0
	}
};


AM_CAP_DETAILS base_cap_details[1] = 
{
 //Descriptors are 64 bytes 
	
	//             1         2         3         4         5         6
	//   01234567890123456789012345678901234567890123456789012345678901234
	{
		AM_TYPE_BASE_APPMEM,
		"Base Appmem",
		0,
		"Brandon's Appmem Device",
		1,
		{
		"Nothing...."
		}
	}

};

static AM_FUNCTION_ENTRY am_base_function_entry =
{
	AM_TYPE_BASE_APPMEM,
	NULL,
	&virtd_base_cap,
    &base_cap_details[0]
};




#endif

