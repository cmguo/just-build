## 项目类型

LOCAL_TYPE			:= proj

## 该项目默认的配置

LOCAL_CONFIG			:= debug

## 项目类型

PROJECT_TYPE			:= bin

## 工程默认的配置

PROJECT_CONFIG			:= multi

## 生成目标名称（完整文件名称要根据类型LOCAL_TYPE、配置PROJECT_CONFIG、版本PROJECT_VERSION增加前缀、后缀）

PROJECT_TARGET			:=

## 项目版本号（只要前三位，最后一位自动生成）

PROJECT_VERSION			:=

## 定义项目版本号的头文件

PROJECT_VERSION_HEADER		:=

## 指定源文件目录（该项目源文件的总目录，相对于根目录ROOT_DIRECTORY，默认为LOCAL_NAME）

PROJECT_SOURCE_DIRECTORY	:= 

## 如果源码目录有子目录，指定子目录的名称（没有指定时，将自动搜索子目录）

PROJECT_SOURCE_SUB_DIRECTORYS	:= 

## 指定搜索源码子目录的深度（默认为1）

PROJECT_SOURCE_DEPTH   		:=

## 项目预编译头文件

PROJECT_COMMON_HEADERS  	:=

## 额外包含目录（多个，相对于根目录ROOT_DIRECTORY）

PROJECT_EXTERNAL_INCLUDES	:=

## 该项目特点的编译选项

PROJECT_COMPILE_FLAGS		:= $(PROJECT_COMPILE_FLAGS)

## 该项目特点的链接选项

PROJECT_LINK_FLAGS		:= $(PROJECT_LINK_FLAGS)

## 该项目依赖的其他项目

PROJECT_DEPENDS			:= $(PROJECT_DEPENDS)

## 该项目特定的引用库

PROJECT_DEPEND_LIBRARYS		:= $(PROJECT_DEPEND_LIBRARYS)
