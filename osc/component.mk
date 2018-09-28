#
# Component Makefile
#

COMPONENT_DEPENDS := \
	flatbuffers \
	uuid

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/tinyosc

COMPONENT_SRCDIRS := \
	src \
	lib/tinyosc

CXXFLAGS += -std=c++14

lib/tinyosc/tinyosc.o: CFLAGS += \
	-include "${COMPONENT_PATH}/src/htonll_ntohll.h" \
	-Wno-overflow
