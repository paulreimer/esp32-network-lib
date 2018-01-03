#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	curl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	. \
	curl/lib \
	curl/include \
	zlib

COMPONENT_SRCDIRS := \
	. \
	curl/lib \
	curl/lib/vauth \
	curl/lib/vtls \
	zlib

CFLAGS += \
	-DHAVE_CONFIG_H=1 \
	-DBUILDING_LIBCURL=1
