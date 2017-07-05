################################################################################
## @file:	version.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	自动版本号.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/macro.mk
include $(ROOT_MAKE_DIRECTORY)/func/repo.mk

ifeq ($(PROJECT_VERSION),)
        ifneq ($(PROJECT_VERSION_HEADER),)
		VERSION			:= $(call get_macro_info,$(PROJECT_VERSION_HEADER),VERSION)

                ifeq ($(VERSION),)
                        $(error VERSION not defined in $(PROJECT_VERSION_HEADER))
                endif
	
		VERSION_NAME		:= $(call get_macro_info,$(PROJECT_VERSION_HEADER),NAME)
        else
		CONFIG_build_version	:= no
        endif
else
	CONFIG_build_version	:= no
	VERSION			:= $(PROJECT_VERSION)
endif

NAME_VERSION		:= $(VERSION)

ifneq ($(VERSION),)
	VERSION_MAJOR		:= $(shell echo $(VERSION) | cut -d . -f 1)
	NAME_VERSION		:= -$(VERSION)
ifneq ($(VERSION),$(VERSION_MAJOR))
	NAME_VERSION_MAJOR	:= -$(VERSION_MAJOR)
endif
endif

ifeq ($(BUILDABLE)$(CONFIG_build_version),yesyes)

        ifeq ($(VERSION_NAME),)
                $(error VERSION_NAME not defined in $(VERSION_HEADER))
        endif
	
	BUILD_VERSION		:= $(if $(VERSION_TYPE)$(BUILD_NUMBER),$(VERSION_TYPE)_$(BUILD_NUMBER),$(call repo_version,$(SOURCE_DIRECTORY)))

	BUILD_VERSION		:= $(strip $(BUILD_VERSION))

	BUILD_VERSION_FILE	:= build_version

        ifneq ($(BUILD_VERSION),$(shell cat $(BUILD_VERSION_FILE) 2> /dev/null))
                $(shell echo -n $(BUILD_VERSION) > $(BUILD_VERSION_FILE))
        endif

endif
