set(srcs "./components_test.c")
set(include_dirs "./INC")

idf_component_register(SRCS "components_test.c" "${srcs}"
                       INCLUDE_DIRS "${include_dirs}")

# REQUIRES      xxx     # 声明public依赖，可供其他依赖者访问。
# PRIV_REQUIRES xxx     # 声明private依赖，只能被当前组件访问。