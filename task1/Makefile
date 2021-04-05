obj-m += phonebook_module.o

all: build
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod phonebook_module.ko

build:
	gcc main.c -fsanitize=address -g

clean:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod phonebook_module.ko
	rm main