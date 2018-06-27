#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/yajl/include

COMPONENT_PRIV_INCLUDEDIRS := \
	lib \
	lib/yajl

COMPONENT_SRCDIRS := \
	src \
	lib/yajl/src

src/json_emitter.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
