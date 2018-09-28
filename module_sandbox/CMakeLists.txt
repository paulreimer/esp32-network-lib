#set(
#  COMPONENT_SRCS
#    #"mod1.c"
#    "mod1.cpp"
#)
#
register_component()
#
#set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD 14)
#set_property(TARGET "${COMPONENT_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)

#MODULE_GENERATE_ELF(mod1_elf mod1.c)
MODULE_GENERATE_ELF(mod1_elf mod1.cpp)
#add_dependencies(${COMPONENT_NAME} mod1_elf_TARGET)
