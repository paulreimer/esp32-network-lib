#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/cpp17_headers/include \
	lib/delegate \
	lib/flatcc/include \
	lib/GSL/include \
	lib/curl/include \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/curl/lib \
	lib/curl/include \
	lib/flatcc \
	lib/yajl \
	lib/zlib

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib \
	lib/curl/lib \
	lib/curl/lib/vauth \
	lib/curl/lib/vtls \
	lib/flatcc/src/runtime \
	lib/yajl/src \
	lib/zlib

CFLAGS += \
	-DHAVE_CONFIG_H=1 \
	-DBUILDING_LIBCURL=1 \
	-DCURL_MAX_WRITE_SIZE=1024 \
	-DCURL_SOCKET_HASH_TABLE_SIZE=17 \
	-DCURL_CONNECTION_HASH_SIZE=17

# Depends on oidc.fbs
src/oidc_token_parser.o: \
	$(COMPONENT_PATH)/src/gen/flatbuffers_common_reader.h \
	$(COMPONENT_PATH)/src/gen/oidc_json_parser.h \
	$(COMPONENT_PATH)/src/gen/oidc_verifier.h \
	$(COMPONENT_PATH)/src/gen/oidc_reader.h

# Output these files to the component path
$(COMPONENT_PATH)/src/gen/flatbuffers_common_reader.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -cv --json-parser -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_json_parser.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -cv --json-parser -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_verifier.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -cv --json-parser -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_reader.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -cv --json-parser -o $(COMPONENT_PATH)/src/gen $^
