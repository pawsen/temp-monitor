# makefiles example
# https://github.com/ladislas/Bare-Arduino-Project/blob/master/Makefile-Linux.mk

# make help
# make upload
# make monitor
# make show_board
# make show_submenu

TARGET=uno
# TARGET=micro
ifeq ($(TARGET), uno)
    MONITOR_PORT = /dev/ttyACM0
    BOARD_TAG = uno
else
    MONITOR_PORT = /dev/ttyUSB0
    BOARD_TAG = pro
    BOARD_SUB = 16MHzatmega328
endif

MONITOR_BAUDRATE  = 115200


include $(ARDMK_DIR)/Arduino.mk
