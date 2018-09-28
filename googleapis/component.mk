#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model \
	embedded_files \
	requests \
	uuid

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen

COMPONENT_SRCDIRS := \
	src \
	src/gen

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/assets/gen/spreadsheet_insert_row_request_intent.req.fb \
	$(COMPONENT_PATH)/assets/gen/visualization_query_request_intent.req.fb \
	$(COMPONENT_PATH)/src/gen/sheets_generated.h \
	$(COMPONENT_PATH)/src/gen/visualization_generated.h

CXXFLAGS += -std=c++14

src/googleapis.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/googleapis.o: $(COMPONENT_PATH)/src/gen/sheets_generated.h
src/googleapis.o: $(COMPONENT_PATH)/src/gen/visualization_generated.h
src/spreadsheet_insert_row_actor.o: $(COMPONENT_PATH)/src/gen/sheets_generated.h
src/spreadsheet_insert_row_actor.o: $(COMPONENT_PATH)/assets/gen/spreadsheet_insert_row_request_intent.req.fb
src/visualization_query_actor.o: $(COMPONENT_PATH)/src/gen/visualization_generated.h
src/visualization_query_actor.o: $(COMPONENT_PATH)/assets/gen/visualization_query_request_intent.req.fb
