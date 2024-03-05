# 目录说明

```
 ├──project_template				// 项目根目录
 	├── CMakeLists.txt
    ├── main						// main目录(组件)，自动包含，名字不可改
    ├── CMakeLists.txt				// main组件注册文件
    │   └── main.c					// main组件文件，名字可改
    └── README.md
    ├── components					// 自定义组件根目录，自动包含，名字一般不可改
    	├── components_template	    // 自定义组件目录1
        	├── CMakeLists.txt		// 组件注册文件
        	├── INC
            │   └── xxx.h
        	├── SRC
            │   └── xxx.c
```

