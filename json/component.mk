#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model \
	requests

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/yajl

COMPONENT_SRCDIRS := \
	src \
	lib/yajl/src

CXXFLAGS += -std=c++14

src/json_emitter.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
