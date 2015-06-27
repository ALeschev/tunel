obj-m += source/rtpsw.o

all:
	make -C /usr/src/linux-headers-`(uname -r)` SUBDIRS=$(PWD) modules

clean:
	make -C /usr/src/linux-headers-`(uname -r)` SUBDIRS=$(PWD) modules clean