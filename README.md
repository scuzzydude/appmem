### Appmem 

This project is a software implementation of an Appmem Device.   An appmem device is a HW device that provides applications with data structures external data structures.   See xxxx for an overview of the concept.
 
The project consists of the following modules.
*  Appmemtest test utility.  Tests both C and C++ interfaces to appmemlib.
*  Appmemlib – shared source module with interfaces and basic data structures.  User mode (emulation) target.   Used by appmemtest, appmemk and am_targ.
*  Appmemk – kernel driver with Memory Mapped and IOCTL interfaces and kernel emulated target.
*	Am_targ – very basic networked target

###  Environment.
I’ve tested the user mode code (everything but appmemk) on 
* x86         
   - CentOS release 6.4 (Linux 2.6.32-358.el6.x86_64 x86_64)
   - Windows7  (am_net/appmemtest network localhost loopback doesn’t work)

* ARMv61  (Raspberry Pi)
   - Raspbian GNU/Linux 7 (wheezy)(Linux 3.6.11+ armv6l)

* appmemk (kernel driver) 
   - CentOS release 6.4 (Linux 2.6.32-358.el6.x86_64 x86_64)
   - appmem.ko doesn’t build on Raspbian yet, need to work out some makefile/kernel header stuff, I don’t believe there is any fundamental reason this won’t work.

### Populate Source Tree
````
git clone https://github.com/scuzzydude/appmem.git
````
### Build
You should have gcc and kernel mode build set up.  If not, that's beyond the scope of this README.  You can figure it out.
````
>>> cd appmem
>>> make
>>> cd am_targ
>>> make
>>> cd ..
````
That's all thats required for the User Mode.
the appmemk kernel driver shares the appmemlib, but there are some compile time difference (malloc/kmalloc, etc..).  So right now, you have to clean to get the build right.  I'm sure this can be worked out with different kernel/user OBJ directories, but I'm a makefile idiot.   If there is a makefile wizard out there who wants to help, please do.

To build the appmem.ko driver
````
>>> cd appmemk
>>> make clean
>>> make
````


### Usage
•	appmemtest
````
>>> appmemtest
appmemtest <appmemdevice> <amType> <test elements> <random ops> <C/C++/Both>

<appmemdevice>
virtd             - Userspace Library Emulation
/dev/appmem       - Kernel Driver Emulation
xxx.xxx.xxx.xxx   - IP of an am_targ

<amType>
1                 - Flat Memory
2                 - Static Array
3                 - Associative Array [DEFAULT]

<test elements>
1 - xxxxx(32bit)  - Number of elements to test
                  - actual meaning is amType specific
                  - For example, in Flat Memory, this in number of bytes
                  - Static Array, this in number of elements
1024 [DEFAULT]    - Associative Array, this in number of keys

<random ops>
1 - xxxxx(32bit)  - Number of random read operations
1000 [DEFAULT]    -
<C/C++/Both>
1                 - C Tests Only
2                 - C++ Tests Only
3                 - Both tests [DEFAULT]
````
*  am_targ
This will start the am_tart net client.   It will listen and respond on UDP port 4950.  On Linux, you can run appmemtest on the same machine at 127.0.0.1.

````
>>> am_targ
````
