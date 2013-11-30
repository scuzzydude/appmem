### Appmem 

This project is a software implementation of an Appmem Device.   An appmem device is a HW device that provides applications with data structures external data structures.   See [Appmem Overview](https://github.com/scuzzydude/appmem/blob/master/doc/Appmem_overview.md) or the [slides] [Appmem Overview - Slides](http://scuzzydude.github.io/appmem/Appmem_overview_slides.pdf) for an intro to the project.
 
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
You should have gcc and kernel headers set up.  If not, that's beyond the scope of this README.  You can figure it out.
````
>>> cd appmem
>>> make
>>> cd am_targ
>>> make
>>> cd ..
````
That's all thats required for the User Mode items.
The appmemk kernel driver shares the appmemlib, but there are some compile time difference (malloc/kmalloc, etc..).  So right now, you have to clean to get the build right.  I'm sure this can be worked out with different kernel/user OBJ directories, but I'm a makefile idiot.   If there is a makefile wizard out there who wants to help, please do.

To build the appmem.ko driver
````
>>> cd appmemk
>>> make clean
>>> make
````


### Usage
*	appmemtest
This will bring up the command line help.  (See Examples)
````
>>> ./appmemtest
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
There is no valuable output from this program unless you enable AM_DEBUGPRINT.  <CTRL-C> To exit. 

````
>>> cd am_targ
>>> ./am_targ
````

*  appmemk 
This will install the kernel emulation of Appmem device at /dev/appmem.   This device can be used to test the kernel interface with appmemtest.
You need to be root or have su rights to install the kernel module.

````
>>> cd appmemk
>>> su
>>> insmod appmem.ko
````

You can check installation and debug messages (if enabled) by using dmesg.

````
>>> dmesg
````

To remove the module

````
>>> rmmod appmem
````

### What the test does
Currently, 3 amTypes are supported: Flat Memory, Static Array, and Associative Array.

The test will perform a series of Writes/Random Reads to the desired first in "native" (raw) mode - i.e. directly. 

Then the test will use the selected device (virtd, /dev/appmem, or am_targ) and perform the same test.

It compares the elapsed times.

Depending on option choosen, it will perform the tests using the C interface and the C++ Class interface.
 
It will also perform offload algorithms like array sort or Iterate an associative array.

At this point, none of the options are optimized and this is purely a test of the code concept.  

The idea is that this will be a benchmark test in the future with real devices.

### Examples

Run The appmem Static Array virtd target 100000 writes and 5000 reads using C++ interface

````
>>> ./appmemtest virtd 2 10000 5000 2 
````

Run the Appmem Associative Array appmemk target 1000 writes and 200 reads using C interface

````
>>> ./appmemtest /dev/appmem 3 1000 200 1
````

Run the Appmem Flat Mem function on remote am_targ at IP Address 192.168.1.221,  100 writes, 200 reads using C.
Must start am_targ on the remote machine
 
```` 
>>> ./appmemtest 192.168.1.221 1 100 200 1
````

### Reference
[Appmem Overview - Slides](http://scuzzydude.github.io/appmem/Appmem_overview_slides.pdf)

[Appmem Overview](https://github.com/scuzzydude/appmem/blob/master/doc/Appmem_overview.md)

[API and Implementation Notes](https://github.com/scuzzydude/appmem/blob/master/doc/API_Implementation_notes.md)


### Author

Brandon Awbrey - scuzzydude@hotmail.com   




