TARGET = daisy_delay
CPP_SOURCES = src/main.cpp src/delay.cpp src/controls.cpp

LIBDAISY_DIR ?= libDaisy
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
APP_TYPE = BOOT_NONE

include $(SYSTEM_FILES_DIR)/Makefile
