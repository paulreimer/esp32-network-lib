#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/sole

COMPONENT_SRCDIRS := \
	src

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/uuid_generated.h

src/uuid.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
