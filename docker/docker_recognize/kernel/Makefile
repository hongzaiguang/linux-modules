KVERS = $(shell uname -r)
obj-m += docker_kernel.o
build: kernel_modules
EXTRA_CFLAGS=-g -o0
kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
