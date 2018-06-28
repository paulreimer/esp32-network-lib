#
# Component Makefile
#

COMPONENT_DEPENDS := \
	flatbuffers

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/tinyosc

COMPONENT_SRCDIRS := \
	src \
	lib/tinyosc

lib/tinyosc/tinyosc.o: CFLAGS += \
	-D'htonll(x)=((((uint64_t)htonl(x) & 0xFFFFFFFF) << 32) + htonl((x) >> 32))' \
	-D'ntohll(x)=((((uint64_t)ntohl(x) & 0xFFFFFFFF) << 32) + ntohl((x) >> 32))'
