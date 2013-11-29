CPPFLAGS=-Iinc -Iinc/ut-hash -g
CC=gcc
all: am_api

#%.o:	%.c
#	  $(CC) $(CPPFLAGS)	-c	-o	$@	$^

am_api:	api.o appmemlib/appmemcpp.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o appmemlib/am_assca.o appmemlib/appmem_kd.o appmemlib/am_linux_net.o appmemlib/appmem_net.o appmemlib/appmem_pack.o
#	 $(CC) -lrt -o $@ $^
	 $(CXX) -lrt -o $@ $^

clean:
	rm -rf *.o am_api appmemlib/*.o
