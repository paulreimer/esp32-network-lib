#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	src/gen

COMPONENT_SRCDIRS := \
	src \
	src/gen

src/googleapis.o: CXXFLAGS += -D_GLIBCXX_USE_C99=1
