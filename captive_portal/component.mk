#
# Component Makefile
#

COMPONENT_DEPENDS := \
	actor_model \
	network_manager

COMPONENT_ADD_INCLUDEDIRS := \
	src

COMPONENT_PRIV_INCLUDEDIRS := \
	lib/dns_server

COMPONENT_SRCDIRS := \
	src \
	lib/dns_server
