set(srcs "components_test.c")
set(include_dirs "./INC")

idf_component_register(SRCS "components_test.c" "${srcs}"
                       INCLUDE_DIRS "${include_dirs}")
