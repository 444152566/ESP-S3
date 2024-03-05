set(srcs "magicLight_main.c")
set(include_dirs ".")

# 组件注册
idf_component_register(
    SRCS            "${srcs}"                  # 源文件
    INCLUDE_DIRS    "${include_dirs}"          # 头文件(目录)
    # PRIV_REQUIRES   mqtt
    # REQUIRES esp_hid
    # PRIV_REQUIRES nvs_flash
    )
    # REQUIRES      xxx                        # 声明public依赖，可供其他依赖者访问     #一般是链接系统组件才用，按指定路径添加的自定义组件会自动链接
    # PRIV_REQUIRES xxx                        # 声明private依赖，只能被当前组件访问。   #一般是链接系统组件才用，按指定路径添加的自定义组件会自动链接
