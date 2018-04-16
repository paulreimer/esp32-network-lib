#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/cpp17_headers/include \
	lib/delegate \
	lib/GSL/include \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/yajl \
	lib/zlib

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib \
	lib/yajl/src \
	lib/zlib

src/json_emitter.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/request_handler.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
