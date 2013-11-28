################################################################################
## @file: 	config.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	工程项目的配置.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/name.mk

CONFIG_PROFILE_LIST	:= debug release

CONFIG_THREAD_LIST	:= multi single

CONFIG_LIB_LIST		:= static dynamic static2

CONFIG			:= $(CONFIG) $(PROJECT_CONFIG)

CONFIG_PROFILE		:= $(call get_config,$(CONFIG),$(CONFIG_PROFILE_LIST),debug)

CONFIG_THREAD		:= $(call get_config,$(CONFIG),$(CONFIG_THREAD_LIST),multi)

TARGET_DIRECTORY	:= $(CONFIG_PROFILE)

ifeq ($(CONFIG_PROFILE),debug)
	NAME_CONFIG		:= gd
	CONFIG_MACROS		:= $(CONFIG_MACROS) _DEBUG
else
	CONFIG_MACROS		:= $(CONFIG_MACROS) _NDEBUG
endif

ifeq ($(PROJECT_TYPE2),lib)

	CONFIG_MACROS		:= $(CONFIG_MACROS) _LIB

	CONFIG_LIB		:= $(call get_config,$(CONFIG),$(CONFIG_LIB_LIST),static)

	TARGET_DIRECTORY	:= $(TARGET_DIRECTORY)/$(CONFIG_LIB)

        ifneq ($(CONFIG_LIB),dynamic)

		NAME_CONFIG		:= s$(NAME_CONFIG)

		NAME_PREFIX		:= $(STATIC_NAME_PREFIX)

		NAME_SUFFIX		:= $(STATIC_NAME_SUFFIX)

		CONFIG_MACROS		:= $(CONFIG_MACROS) _STATIC

        else

		NAME_PREFIX		:= $(DYNAMIC_NAME_PREFIX)

		NAME_SUFFIX		:= $(DYNAMIC_NAME_SUFFIX)

		CONFIG_MACROS		:= $(CONFIG_MACROS) _DYNAMIC

        endif

else

	CONFIG_MACROS		:= $(CONFIG_MACROS) _BIN

	NAME_PREFIX	:= $(BIN_NAME_PREFIX)

	NAME_SUFFIX	:= $(BIN_NAME_SUFFIX)

endif

ifneq ($(NAME_CONFIG),)
	NAME_CONFIG		:= -$(NAME_CONFIG)
endif

ifeq ($(CONFIG_THREAD),multi)
	NAME_CONFIG		:= -mt$(NAME_CONFIG)
	CONFIG_MACROS		:= $(CONFIG_MACROS) _MULTI
else
	CONFIG_MACROS		:= $(CONFIG_MACROS) _SINGLE
endif
