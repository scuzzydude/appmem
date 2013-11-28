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
   CentOS release 6.4
   Windows7  (am_net/appmemtest network localhost loopback doesn’t work)

*ARMv61 
   Raspbian GNU/Linux 7 (wheezy)

*I’ve tested appmemk on 
   CentOS release 6.4 (Linux 2.6.32-358.el6.x86_64 x86_64)
   appmem.ko doesn’t build on Raspbian yet, need to work out some makefile/kernel header stuff, I don’t believe there is any fundamental reason this won’t work.

