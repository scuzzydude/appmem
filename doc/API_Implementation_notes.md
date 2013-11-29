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
  -  