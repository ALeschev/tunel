obj-m += keypadModule.o

all:
	make -C /usr/src/linux-headers-`(uname -r)` SUBDIRS=$(PWD) modules

clean:
	make -C /usr/src/linux-headers-`(uname -r)` SUBDIRS=$(PWD) modules clean