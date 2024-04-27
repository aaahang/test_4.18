
KERN_DIR = /home/stoicus/imx6ull/bsp/100ask_imx6ull-sdk/Linux-4.9.88
INCDIR := /home/stoicus/imx6ull/source/test_learning_noqemu/test_4.18
TARGET :=  test_timer
obj-m = $(TARGET).o
EXTRA_CFLAGS += -I$(INCDIR)
$(TARGET)-objs := /drv/irq_test_drv.o

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(CROSS_COMPILE)gcc -o test ./app/test.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f  test
