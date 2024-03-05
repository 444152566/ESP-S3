set(srcs "template_main.c")
set(include_dirs ".")

# 组件注册
idf_component_register(
    SRCS            "${srcs}"                  # 源文件
    INCLUDE_DIRS    "${include_dirs}"          # 头文件(目录)
    )

# 关于REQUIRES组件依赖和PRIV_REQUIRES组件依赖，由于main组件会自动依赖所有组件，所以不需在这里手动添加
