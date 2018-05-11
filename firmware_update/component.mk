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
# Depends on top-level VERSION file contents
src/firmware_update.o: $(PROJECT_PATH)/VERSION
src/firmware_update_actor.o: $(COMPONENT_PATH)/src/gen/firmware_update_generated.h
