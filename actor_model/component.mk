#
# Component Makefile
#

COMPONENT_DEPENDS := \
	esp_ringbuf \
	uuid

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/delegate \
	lib/simple_match/include

COMPONENT_SRCDIRS := \
	src

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/actor_model_generated.h

src/actor.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/actor_model.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/mailbox.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/node.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/node.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/pid.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/received_message.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
