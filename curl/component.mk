#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	lib/curl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/curl/lib \
	lib/curl/include \
	lib/zlib

COMPONENT_SRCDIRS := \
	lib \
	lib/curl/lib \
	lib/curl/lib/vauth \
	lib/curl/lib/vtls \
	lib/zlib

CFLAGS += \
	-DHAVE_CONFIG_H=1 \
	-DBUILDING_LIBCURL=1 \
	-DCURL_MAX_WRITE_SIZE=1024 \
	-DCURL_SOCKET_HASH_TABLE_SIZE=3 \
	-DCURL_CONNECTION_HASH_SIZE=3

lib/curl/lib/http2.o: CFLAGS += -Wno-unused-variable
lib/curl/lib/vtls/mbedtls.o: CFLAGS += -Wno-unused-variable
