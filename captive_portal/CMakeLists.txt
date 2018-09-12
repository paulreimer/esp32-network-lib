set(COMPONENT_REQUIRES actor_model network_manager)

set(COMPONENT_ADD_INCLUDEDIRS src)
set(COMPONENT_PRIV_INCLUDEDIRS lib/dns_server)

set(
  COMPONENT_SRCS
    "lib/dns_server/dns_server.cpp"
    "src/captive_portal.cpp"
    "src/dns_server_actor.cpp"
)

register_component()
