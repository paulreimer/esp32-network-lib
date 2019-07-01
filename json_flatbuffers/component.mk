#
# Component Makefile
#

COMPONENT_DEPENDS := \
	flatbuffers \
	json_streaming

COMPONENT_ADD_INCLUDEDIRS := \
	src

COMPONENT_SRCDIRS := \
	src \
	lib/flatbuffers/src

CXXFLAGS += -std=c++14
