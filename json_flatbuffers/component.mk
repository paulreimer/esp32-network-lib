#
# Component Makefile
#

COMPONENT_DEPENDS := \
	flatbuffers \
	requests

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/yajl

COMPONENT_SRCDIRS := \
	src \
	lib/flatbuffers/src \
	lib/yajl/src

src/json_emitter.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/json_emitter.o: $(PROJECT_PATH)/esp32-network-lib/requests/src/gen/requests_generated.h
