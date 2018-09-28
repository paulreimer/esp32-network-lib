#
# Component Makefile
#

COMPONENT_DEPENDS := \
	flatbuffers

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/sole

COMPONENT_SRCDIRS := \
	src

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/uuid_generated.h

CXXFLAGS += -std=c++14

src/uuid.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
