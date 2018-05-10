#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/cpp17_headers/include \
	lib/delegate \
	lib/GSL/include \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/yajl

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib \
	lib/yajl/src

COMPONENT_EMBED_FILES := \
	src/gen/requests.bfbs

src/requests.o: $(COMPONENT_PATH)/src/gen/requests.bfbs
src/json_emitter.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/json_emitter.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_handler.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
