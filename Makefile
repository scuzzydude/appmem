CPPFLAGS=-Iinc -Iinc/ut-hash -g
CC=gcc
all: appmemtest

#%.o:	%.c
#	  $(CC) $(CPPFLAGS)	-c	-o	$@	$^

appmemtest:	appmem_test.o amcpptest.o appmemlib/appmemcpp.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o appmemlib/am_assca.o appmemlib/appmem_kd.o
#appmemtest:	appmem_test.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o appmemlib/am_assca.o appmemlib/appmem_kd.o
#	 $(CC) -lrt -o $@ $^
	 $(CXX) -lrt -o $@ $^

clean:
	rm -rf *.o appmemtest appmemlib/*.o
