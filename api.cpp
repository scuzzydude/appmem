
#include <cstdlib>
#include <stdio.h>
#include "appmemcpp.h"

#include "am_test_os.h"

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
