################################################################################
## @file:	paths.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	处理目录细节
## @version	1.0
###############################################################################

INC_PATHS		:= $(INC_PATHS) $(wildcard $(addsuffix include,$(DEPEND_PATHS)))
INC_PATHS		:= $(INC_PATHS) $(HEADER_DIRECTORYS)
INC_PATHS		:= $(INC_PATHS) $(PLATFORM_INCLUDE_DIRECTORYS)
INC_PATHS		:= $(INC_PATHS) $(addprefix $(ROOT_DIRECTORY)/,$(GLOBAL_INCLUDE_DIRECTORYS))

INC_PATHS		:= $(call uniq,$(INC_PATHS))

# 获取所有 pkgconfig 目录
PKG_CONFIG_DIRS := $(wildcard $(addsuffix lib/pkgconfig,$(DEPEND_PATHS)))
PKG_CONFIG_DIRS := $(call uniq,$(PKG_CONFIG_DIRS))
PKG_CONFIG_DIRS := $(foreach dir,$(PKG_CONFIG_DIRS),$(realpath $(dir)))
#$(warning $(PKG_CONFIG_DIRS))

# 使用":"连接所有 pkgconfig 目录
noop=
space = $(noop) $(noop)
PKG_CONFIG_DIRS := $(subst $(space),:,$(PKG_CONFIG_DIRS))
#$(warning $(PKG_CONFIG_DIRS))

# 获取所有lib
PKG_CONFIG_FILES := $(wildcard $(addsuffix lib/pkgconfig/*,$(DEPEND_PATHS)))
PKG_CONFIG_ALL_LIBS = $(foreach config_file,$(PKG_CONFIG_FILES),$(basename $(notdir $(config_file))))
#$(warning $(PKG_CONFIG_ALL_LIBS))

PKG_CFLAGS := $(foreach l,$(PKG_CONFIG_ALL_LIBS),$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_DIRS) pkg-config --cflags-only-I $(l)))
#$(warning $(PKG_CFLAGS))
PKG_CONFIG_INC_PATHS := $(subst -I,$(noop),$(PKG_CFLAGS))
PKG_CONFIG_INC_PATHS := $(call uniq,$(PKG_CONFIG_INC_PATHS))
#$(warning $(PKG_CONFIG_INC_PATHS))

PKG_CONFIG_LIB_PATHS := $(foreach l,$(PKG_CONFIG_ALL_LIBS),$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_DIRS) pkg-config --libs-only-L $(l)))
PKG_CONFIG_LIB_PATHS := $(subst -L,$(noop),$(PKG_CONFIG_LIB_PATHS))
PKG_CONFIG_LIB_PATHS := $(call uniq,$(PKG_CONFIG_LIB_PATHS))
#$(warning $(PKG_CONFIG_LIB_PATHS))

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_PATHS		:= $(LIB_PATHS) $(DEPEND_PATHS)

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)

LIB_PATHS2		:= $(filter %/,$(DEPEND_FILES))
LIB_PATHS2		:= $(call joinlist,:,$(strip $(LIB_PATHS2) $(LD_LIBRARY_PATH)))

endif # dynamic, bin

endif # dynamic static2, bin

LIB_PATHS := $(LIB_PATHS) $(PKG_CONFIG_LIB_PATHS)

LIB_PATHS		:= $(call uniq,$(LIB_PATHS))