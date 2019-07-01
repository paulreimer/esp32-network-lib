#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model \
	curl \
	embedded_files \
	json_streaming \
	nghttp \
	uuid

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/cpp17_headers/include \

COMPONENT_PRIV_INCLUDEDIRS := \
	lib

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib

COMPONENT_EMBED_FILES := \
	src/gen/requests.bfbs

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/requests.bfbs \
	$(COMPONENT_PATH)/src/gen/requests_generated.h

CXXFLAGS += -std=c++14

src/requests.o: $(COMPONENT_PATH)/src/gen/requests.bfbs
src/request_handler.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
