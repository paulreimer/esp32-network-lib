#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/libelfin

COMPONENT_PRIV_INCLUDEDIRS := \
	lib/libelfin/elf

COMPONENT_SRCDIRS := \
	src \
	src/gen \
	lib/libelfin/elf

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/symbols.c

CXXFLAGS += \
	-std=c++14 \
	-D_GLIBCXX_USE_C99=1

# Module/generation configuration:
PROJECT_NAME ?= makerlabs-acm-reader-lock
APP_ELF ?= $(BUILD_DIR_BASE)/$(PROJECT_NAME).elf

NM ?= $(call dequote,$(CONFIG_TOOLPREFIX))nm

src/module_task.o: src/gen/symbols.o

src/gen/symbols.o: $(COMPONENT_PATH)/src/gen/symbols.c

#		$(NM) $(APP_ELF) | awk -f $(COMPONENT_PATH)/mknmlist > $@; \
#
#$(COMPONENT_PATH)/src/gen/symbols.c: $(APP_ELF)
$(COMPONENT_PATH)/src/gen/symbols.c:
	if [ -f "$(APP_ELF)" ]; then \
		$(NM) $(APP_ELF) | $(COMPONENT_PATH)/gen_symbols_c.py > $@; \
	else \
		: | $(COMPONENT_PATH)/gen_symbols_c.py > $@; \
	fi
