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
src/googleapis.o: $(COMPONENT_PATH)/src/gen/gviz_generated.h
