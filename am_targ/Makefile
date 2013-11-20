CPPFLAGS=-I../inc -I../inc/ut-hash -g
CC=gcc
all: am_targ

#%.o:	%.c
#	  $(CC) $(CPPFLAGS)	-c	-o	$@	$^

am_targ:	am_target.o \
   ../appmemlib/am_linux_net.o \
   ../appmemlib/appmem_net.o \
   ../appmemlib/am_stata.o \
   ../appmemlib/am_flat.o
#appmemtest:	appmem_test.o amcpptest.o appmemlib/appmemcpp.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o appmemlib/am_assca.o appmemlib/appmem_kd.o appmemlib/am_linux_net.o appmemlib/appmem_net.o
#appmemtest:	appmem_test.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o appmemlib/am_assca.o appmemlib/appmem_kd.o
#	 $(CC) -lrt -o $@ $^
	 $(CXX) -lrt -o $@ $^

clean:
	rm -rf *.o am_targ ../appmemlib/*.o
