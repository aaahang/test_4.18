
KERN_DIR = /home/stoicus/imx6ull/bsp/100ask_imx6ull-sdk/Linux-4.9.88
INCDIR := /home/stoicus/imx6ull/source/test_learning_noqemu/test_4.18
TARGET :=  test_drv
obj-m = $(TARGET).o
EXTRA_CFLAGS += -I$(INCDIR)
ccflags-y := -std=gnu11
$(TARGET)-objs :=  topic_drv/get_temdata.o

#设备树编译相关变量
DTS_FILE_NAME :=  ds18b20_fluent_tem.dts#设备树名字
DTS_FILE_PTAH :=  ./topic_drv#设备树路径()
DTS_FILE := $(DTS_FILE_PTAH)/$(DTS_FILE_NAME)
DTB_FILE := $(DTS_FILE:.dts=.dtb)
KERNEL_DTBS_PATH = /home/stoicus/imx6ull/bsp/100ask_imx6ull-sdk/Linux-4.9.88/arch/arm/boot/dts#源码存放设备树的路径
DTB_OUTPUT  = $(KERNEL_DTBS_PATH)/$(DTS_FILE_NAME:.dts=.dtb)

# 在设备树相关的makefile中修改
DTS_MAKEFILE_PAHT := $(KERNEL_DTBS_PATH)/Makefile
all:$(DTB_FILE)
	make -C $(KERN_DIR) M=`pwd` modules 
	# $(CROSS_COMPILE)gcc -o test ./app/test.c -pthread 

dtbs:
	sed -i "/$(DTS_FILE_NAME:.dts=.dtb)/d" $(DTS_MAKEFILE_PAHT)
	sed -i "540i\$(DTS_FILE_NAME:.dts=.dtb)"  $(DTS_MAKEFILE_PAHT) #数字是放在第几行 需要看一下自己的makefile文件 
	cp $(DTS_FILE) $(KERNEL_DTBS_PATH)
	make -C $(KERN_DIR)  dtbs 
	mv $(DTB_OUTPUT) $(DTS_FILE_PTAH)
	rm $(KERNEL_DTBS_PATH)/$(DTS_FILE_NAME)
	sed -i "/$(DTS_FILE_NAME:.dts=.dtb)/d" $(DTS_MAKEFILE_PAHT)
	#这一步是为了让开发板子识别到 boot里面名字确定
	cp $(DTS_FILE_PTAH)/$(DTS_FILE_NAME:.dts=.dtb) 100ask_imx6ull-14x14.dtb 

echo: 
	echo $(DTB_OUTPUT)
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	rm -f  test
