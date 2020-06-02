#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
DEFINES := -DWIFI_SSID=${SSID} -DWIFI_PASS=${PSWD}  
DEFINES += -DESP8266_IDF=1 $(DEFINE) -DNO_ATOMIC 
DEFINES += -DMQTT_HOST=limero.ddns.net -DMQTT_PORT=1883 
SERIAL_PORT = /dev/ttyUSB0
ESPBAUD=921600
MONITORBAUD=115200
CXXFLAGS +=  $(DEFINES) -I/home/lieven/workspace/ArduinoJson/src
CPPFLAGS +=  $(DEFINES) -I/home/lieven/workspace/ArduinoJson/src

PROJECT_NAME := tinyEsp

include $(IDF_PATH)/make/project.mk

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(MONITORBAUD) -C $(TTY)_minicom.log