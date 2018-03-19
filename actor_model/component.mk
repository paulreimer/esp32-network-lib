#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/sole

COMPONENT_SRCDIRS := \
	src

src/actor.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h
src/mailbox.o: $(COMPONENT_PATH)/src/gen/actor_model_generated.h

$(COMPONENT_PATH)/src/gen/actor_model_generated.h: $(COMPONENT_PATH)/actor_model.fbs
	flatc --cpp -o $(@D) \
	--scoped-enums \
	--gen-mutable \
	--gen-object-api \
	--gen-name-strings \
	--reflect-types \
	--reflect-names \
	$^
