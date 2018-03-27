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

# Depends on oidc.fbs
src/oidc_token_parser.o: \
	$(COMPONENT_PATH)/src/gen/flatbuffers_common_builder.h \
	$(COMPONENT_PATH)/src/gen/flatbuffers_common_reader.h \
	$(COMPONENT_PATH)/src/gen/oidc_builder.h \
	$(COMPONENT_PATH)/src/gen/oidc_reader.h \
	$(COMPONENT_PATH)/src/gen/oidc_verifier.h \
	$(COMPONENT_PATH)/src/gen/oidc_json_parser.h \
	$(COMPONENT_PATH)/src/gen/oidc_json_printer.h

src/request_handler.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: $(COMPONENT_PATH)/src/gen/requests_generated.h
src/request_manager.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
