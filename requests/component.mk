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
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/flatcc \
	lib/yajl \
	lib/zlib

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib \
	lib/flatcc/src/runtime \
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

# Output these files to the component path
$(COMPONENT_PATH)/src/gen/flatbuffers_common_builder.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -cw -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/flatbuffers_common_reader.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -c -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_builder.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -w -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_reader.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_verifier.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc -v -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_json_parser.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc --json-parser -o $(COMPONENT_PATH)/src/gen $^
$(COMPONENT_PATH)/src/gen/oidc_json_printer.h: $(COMPONENT_PATH)/src/oidc.fbs
	/Users/paulreimer/Development/lib/mymonster/bin/flatcc --json-printer -o $(COMPONENT_PATH)/src/gen $^
