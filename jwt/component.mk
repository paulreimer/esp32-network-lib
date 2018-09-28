#
# Component Makefile
#

COMPONENT_DEPENDS := \
	base64 \
	mbedtls

COMPONENT_ADD_INCLUDEDIRS := \
	src

COMPONENT_SRCDIRS := \
	src

CXXFLAGS += -std=c++14
