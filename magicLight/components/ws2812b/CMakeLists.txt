set(srcs "light2812.c" 
        "lighteffect2812.c"
        "list.c")
set(include_dirs "./INC")

idf_component_register(SRCS "lighteffect2812.c" "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES freertos
                       PRIV_REQUIRES driver)

