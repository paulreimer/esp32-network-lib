#
# Component Makefile
#

COMPONENT_DEPENDS := \
	app_update \
	mbedtls \
	requests \
	spi_flash

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen

COMPONENT_SRCDIRS := \
	src \
	src/gen

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/firmware_update_generated.h

CXXFLAGS += -std=c++14

# Depends on firmware_update.fbs
src/firmware_update.o: $(COMPONENT_PATH)/src/gen/firmware_update_generated.h
# Depends on top-level VERSION file contents
src/firmware_update.o: $(IDF_PROJECT_PATH)/VERSION
src/firmware_update_actor.o: $(COMPONENT_PATH)/src/gen/firmware_update_generated.h
