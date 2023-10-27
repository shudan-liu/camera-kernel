KBUILD_OPTIONS += CAMERA_KERNEL_ROOT=$(shell pwd)
KBUILD_OPTIONS += KERNEL_ROOT=$(ROOT_DIR)/$(KERNEL_DIR)
KBUILD_OPTIONS += MODNAME=camera

all: modules

CAMERA_COMPILE_TIME = $(shell date)
CAMERA_COMPILE_BY = $(shell whoami | sed 's/\\/\\\\/')
CAMERA_COMPILE_HOST = $(shell uname -n)

cam_generated_h: $(shell find . -iname "*.c") $(shell find . -iname "*.h") $(shell find . -iname "*.mk")
	echo '#define CAMERA_COMPILE_TIME "$(CAMERA_COMPILE_TIME)"' > cam_generated_h
	echo '#define CAMERA_COMPILE_BY "$(CAMERA_COMPILE_BY)"' >> cam_generated_h
	echo '#define CAMERA_COMPILE_HOST "$(CAMERA_COMPILE_HOST)"' >> cam_generated_h
SRC := $(shell pwd)

modules: cam_generated_h
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules $(KBUILD_OPTIONS)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean $(KBUILD_OPTIONS)

headers_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) headers_install $(KBUILD_OPTIONS)
