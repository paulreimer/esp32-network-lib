#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	yajl/include \
	curl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	. \
	curl/lib \
	curl/include \
	yajl \
	zlib

COMPONENT_SRCDIRS := \
	. \
	curl/lib \
	curl/lib/vauth \
	curl/lib/vtls \
	yajl/src \
	zlib

CFLAGS += \
	-DHAVE_CONFIG_H=1 \
	-DBUILDING_LIBCURL=1
