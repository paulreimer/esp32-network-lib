#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/sole \
	lib/simple_match/include

COMPONENT_SRCDIRS := \
	src

src/actor.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/actor.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
src/actor_model.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/actor_model.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
src/mailbox.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/mailbox.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
src/node.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/node.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
src/pid.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/pid.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
src/uuid.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/uuid.o: $(COMPONENT_PATH)/src/gen/uuid_generated.h
