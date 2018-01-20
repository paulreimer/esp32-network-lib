#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	src \
	lib/date/include

COMPONENT_SRCDIRS := \
	src

src/ntp_task.o: CXXFLAGS += \
	-D_GLIBCXX_USE_C99=1
