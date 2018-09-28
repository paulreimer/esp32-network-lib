#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen \
	lib/date/include

COMPONENT_SRCDIRS := \
	src

COMPONENT_EXTRA_CLEAN := \
	$(COMPONENT_PATH)/src/gen/network_manager_generated.h

CXXFLAGS += -std=c++14

src/ntp.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/ntp_actor.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
src/mdns_actor.o: $(COMPONENT_PATH)/src/gen/network_manager_generated.h
src/network_check_actor.o: $(COMPONENT_PATH)/src/gen/network_manager_generated.h
src/ntp_actor.o: $(COMPONENT_PATH)/src/gen/network_manager_generated.h
src/wifi_actor.o: $(COMPONENT_PATH)/src/gen/network_manager_generated.h
