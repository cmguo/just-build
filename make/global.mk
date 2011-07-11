################################################################################
## @file:	global.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	全局设置.
## @version	1.0
###############################################################################

GLOBAL_COMPILE_FLAGS		:= $(GLOBAL_COMPILE_FLAGS) -DBOOST_USER_CONFIG=\<boost_config.h\>
GLOBAL_COMPILE_FLAGS		:= $(GLOBAL_COMPILE_FLAGS) -DFRAMEWORK_USER_CONFIG=\<framework_config.h\>

PLATFORM_LOCAL_NAME		:= $(patsubst $(shell cd $(ROOT_BUILD_DIRECTORY) ; pwd)%,%,$(shell pwd))

PLATFORM_NAME			:= $(firstword $(subst /, ,$(PLATFORM_LOCAL_NAME)))

LOCAL_NAME			:= $(patsubst /$(PLATFORM_NAME)%,%,$(PLATFORM_LOCAL_NAME))

PLATFORM_DIRECTORY		:= $(ROOT_PLATFORM_DIRECTORY)/$(PLATFORM_NAME)

include $(wildcard $(PLATFORM_DIRECTORY)/make/*.mk)

PLATFORM_BUILD_DIRECTORY		:= $(ROOT_BUILD_DIRECTORY)/$(PLATFORM_NAME)
        
PLATFORM_TOOL_DIRECTORY		:= $(ROOT_TOOL_DIRECTORY)/$(PLATFORM_NAME)

PLATFORM_TOOL_PATH		:= $(addprefix $(PLATFORM_TOOL_DIRECTORY),$(PLATFORM_TOOL_PATH))
PLATFORM_TOOL_PATH		:= $(foreach path,$(PLATFORM_TOOL_PATH),$(shell cd $(path) ; pwd))
$(foreach path,$(PLATFORM_TOOL_PATH),$(eval PATH:=$(path):$(PATH)))
export PATH

PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) $(PLATFORM_DIRECTORY)

ifneq ($(wildcard $(PLATFORM_DIRECTORY)/cex/Makefile),)
	PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) $(PLATFORM_DIRECTORY)/cex
        ifneq ($(LOCAL_NAME),/cex)
		PLATFORM_DEPENDS		:= $(PLATFORM_DEPENDS) /cex
        endif
endif

GLOBAL_COMPILE_FLAGS		:= $(GLOBAL_COMPILE_FLAGS) -DPLATFORM_NAME=$(PLATFORM_NAME) -DTOOL_NAME=$(PLATFORM_TOOL_NAME)
GLOBAL_COMPILE_FLAGS		:= $(GLOBAL_COMPILE_FLAGS) -fvisibility=hidden

GLOBAL_LINK_FLAGS		:= $(GLOBAL_LINK_FLAGS) -Wl,--exclude-libs,ALL

include $(ROOT_PROJECT_DIRECTORY)$(LOCAL_NAME)/Makefile.in

parents				= $(1) $(if $(notdir $(1)),$(call parents,$(dir $(1)))))

COMMON_MAKE_FILES		:= $(addprefix $(ROOT_PROJECT_DIRECTORY),$(addsuffix /Common.mk,$(call parents,$(LOCAL_NAME))))

-include $(COMMON_MAKE_FILES)
