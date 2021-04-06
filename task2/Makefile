obj-m += keyboard_spy_module.o

all: build
	sudo insmod keyboard_spy_module.ko

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod keyboard_spy_module.ko
	rm main