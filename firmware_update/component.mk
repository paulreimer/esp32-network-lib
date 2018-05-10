#
# Component Makefile
#

COMPONENT_DEPENDS := \
	requests

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen

COMPONENT_SRCDIRS := \
	src \
	src/gen

# Depends on firmware_update.fbs
src/firmware_update.o: $(COMPONENT_PATH)/src/gen/firmware_update_generated.h
src/firmware_update_actor.o: $(COMPONENT_PATH)/src/gen/firmware_update_generated.h
