
# Comment/uncomment the following line to enable/disable debugging
DEBUG = y


ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -D_APPMEMD -DAM_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2 -D_APPMEMD
endif

EXTRA_CFLAGS += $(DEBFLAGS) -I$(APPMINCDIR) -I$(PWD)/inc -I$(APPMINCDIR)/ut-hash

TARGET = appmemk

ifneq ($(KERNELRELEASE),)


obj-m += appmem.o
appmem-objs := appmemk.o ../appmemlib/am_flat.o ../appmemlib/am_stata.o ../appmemlib/am_assca.o am_k_sock.o \
   ../appmemlib/appmem_pack.o \
   ../appmemlib/funcs/am_list.o
else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
##RaspberryPi -- KERNELDIR ?= /lib/modules/3.6-trunk-rpi/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) APPMINCDIR=$(PWD)/../inc modules

endif


install:
	install -d $(INSTALLDIR)
	install -c $(TARGET).o $(INSTALLDIR)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions ../appmemlib/*.o ../appmemlib/*.ko ../appmemlib/funcs/*.o ../appmemlib/funcs/*.ko

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
