#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model \
	embedded_files \
	jwt \
	mqtt \
	uuid \
	utils

COMPONENT_ADD_INCLUDEDIRS := \
	src/gen \
	src

COMPONENT_SRCDIRS := \
	src

COMPONENT_EMBED_FILES := \
	src/gen/mqtt.bfbs

CXXFLAGS += -std=c++14

src/mqtt_client_actor.o: $(COMPONENT_PATH)/src/gen/mqtt_generated.h
src/mqtt_client_actor.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1

src/mqtt.o: $(COMPONENT_PATH)/src/gen/mqtt_generated.h
src/mqtt.o: $(COMPONENT_PATH)/src/gen/mqtt.bfbs
