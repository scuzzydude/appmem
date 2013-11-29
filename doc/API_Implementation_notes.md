API and Implementation Notes

### Code Organization 

* appmemtest.c 
   - This is main entry point for the test program.   It calls into C interfaces to the appmemlib.
* amcpptest.c 
   - This is the entry point for the tests of the C++ class interface.
   
* /appmemlib
  This is the main interface library.  For user mode, the idea is a either a static or dynamically linked library, but for now, it's just a source module, linked by both appmemtest, am_targ and appmemk.
  - am_assca.c       : Associative Array implementation.
  - am_flat.c        : Flat Memory implementation.
  - am_stata.c       : Static Array implementation.
  - am_linux_net.c   : Linux Socket interface (and threading)
  - appmemlib.c      : Main generic interface to devices.
  - appmem_kd.c      : Interface to the appmemk emulation (ioctl).
  - appmem_net.c     : Network target interface.
  - appmem_pack.c    : Packet interface (net and MMIO appmemk packet)
  - appmem_virtd.c   : Virtual Device (user mode emulation) interface.
  
* /am_targ
 Target Client software.
  - am_target.c      : Target implementation.   Ideally, this should be a good template for any type target, but for now, it UPD packet based.

* /appmemk
 Appmem.ko Kernel Emulation Device implementation.
  - appmemk.c - Driver entry point and command handler.  
  - am_k_sock.c - Kernel mode socket driver (incomplete).
* /inc
  - headers (need better organization here, work in progress and subject to change).
  
* /inc/ut-hash
  - ut-hash from http://troydhanson.github.com/uthash/, thanks to Troy Hanson for easy to use and simple hash implementation.  This by am_assca.c, and modified slightly from original to work in kernel.
  
* /win 
   helper code for Windows build.   I'm not distributing any makefiles or project files for windows, it only works with the user mode stuff, and since my primary target device will require a kernel driver, I may stop testing this at any time.  However, if anybody is interested I can tell you how I build it.
   
   
   
### API 

* Applicaton API 

It's early and subject to change.  I'm not real happy with the terminology.
Rather than a detailed description, I'll just refer you to appmemtest.c and give a short walk through of the C++ process, which abstracts much of the complexity of the C code.
A note on the C++ interface - I'm a firmware/driver guy, and haven't done any serious C++ coding since 1999.  So what I have might be lame.  I need to work with some real "big data" applications coders to refine both the interfaces and the use cases.

See api.cpp 

````C++
int main(void)
{
	//Driver name is NULL = virtd (Virtual Emulation Driver)
	//or /dev/appmem  (Kernel Emulation Driver)
	//or IP Address of remote am_targ
	char * am_name = NULL;  
	char key[] = "0123456789ABCDE";
	unsigned int key_val = 0xBADA5DAD;
	unsigned int read_val = 0;
	
	//Create an Associative Array with fixed key length of 16 bytes
	//and a fixed data length of 4 bytes 
	CAppMemAsscArray amAssca(am_name, 16, sizeof(unsigned int), true, true);

	//Check to make sure the Device is ready 
	//i.e. communiction has been established through the transport 
	if(amAssca.ready())
	{

		//Insert the key and value to the device Array 
		amAssca.insert(key, &key_val);		


		//Read the key back from the device
		if(AM_RET_GOOD == amAssca.get(key, &read_val))
		{
			printf("Key %s Read Value = 0x%08x\n", key, read_val);
		}
		else
		{
			printf("Key Read Error\n");
		}
		

	}

	//Destructor should close the device function 
	//and the just like a local array
	//the memory is gone.  However, we may have options for persistance
	//and we'd use a dyncamically allocated object if we needed to pass
	//it to another function.
	return 0;
}
````

 
  
  