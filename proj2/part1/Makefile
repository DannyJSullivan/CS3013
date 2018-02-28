obj-m := part1.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc test1.c -o test1

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm test1
