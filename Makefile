obj-m += procesos_201403525.o
obj-m += unlink_201403525.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ejecutar_modulos:
	dmesg -C
	insmod procesos_201403525.ko
	rmmod procesos_201403525.ko
	dmesg

