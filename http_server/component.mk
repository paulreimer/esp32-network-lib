#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen

COMPONENT_SRCDIRS := \
	src \
	src/gen

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/http_server_generated.h

src/http_server_actor.o: $(COMPONENT_PATH)/src/gen/http_server_generated.h
