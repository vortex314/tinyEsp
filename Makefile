#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
TTY ?= USB0
undefine __ESP_FILE_ # doesn't seem to work for recursive make
DEFINE ?= -DAAAAA=BBBBBB	# default, cannot be void
DEFINES := -DWIFI_SSID=${SSID} -DWIFI_PASS=${PSWD}  
DEFINES += -DESP8266_IDF=1 $(DEFINE) -DNO_ATOMIC 
DEFINES += -DMQTT_HOST=limero.ddns.net -DMQTT_PORT=1883 
SERIAL_PORT ?= /dev/tty$(TTY)
ESPPORT = $(SERIAL_PORT)
ESPBAUD=921600
MONITORBAUD=115200
CXXFLAGS +=  $(DEFINES) -I/home/lieven/workspace/ArduinoJson/src   -I../../nanoAkka/components/config
CPPFLAGS +=  $(DEFINES) -I/home/lieven/workspace/ArduinoJson/src  -I../../nanoAkka/components/config

PROJECT_NAME := tinyEsp

include $(IDF_PATH)/make/project.mk

SHAKER1 :
	touch main/main.cpp
	make DEFINE=" -DTREESHAKER -DHOSTNAME=shaker1"

SONOFF2 :
	touch main/main.cpp
	make DEFINE=" -DSONOFF -DHOSTNAME=sonoff2"

NODEMCU :
	touch main/main.cpp
	make DEFINE=" -DNODEMCU -DHOSTNAME=nodemcu"


term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(MONITORBAUD) -C $(TTY)_minicom.log