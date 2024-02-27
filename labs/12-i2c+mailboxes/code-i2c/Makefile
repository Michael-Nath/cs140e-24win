# how do we drop it in?

IMU_DIR = mpu6050/

PROGS = $(IMU_DIR)/driver-accel.c

PROGS = 0-test-id.c

COMMON_SRC = i2c.c

COMMON_SRC += $(IMU_DIR)/mpu-6050.c
# STAFF_OBJS = staff-i2c.o

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=1

LIBS += $(LGCC)
LIB_POST := $(LGCC)

include $(CS140E_2024_PATH)/libpi/mk/Makefile.template
