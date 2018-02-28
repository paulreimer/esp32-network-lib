#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src

COMPONENT_SRCDIRS := \
	src

$(COMPONENT_PATH)/src/actor_model_generated.h: $(COMPONENT_PATH)/actor_model.fbs
	flatc --cpp -o $(@D) \
	--scoped-enums \
	--gen-mutable \
	--gen-object-api \
	--gen-name-strings \
	--reflect-types \
	--reflect-names \
	$^
