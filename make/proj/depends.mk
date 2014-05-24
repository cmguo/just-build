################################################################################
## @file: 	depends.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	寻找依赖项.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/info.mk
include $(ROOT_MAKE_DIRECTORY)/func/depends.mk

include $(PROJ_MAKE_DIRECTORY)/func/info.mk

define proj_print
$(2) := $(strip $($(2)) $(1):$(call get_item,$(1),Type):$(call get_item,$(1),File):$(call get_item,$(1),Depends):$(call get_item,$(1),DependLibs)!)
endef

DEPEND_ITEMS		:= $(call tree_visit,$(LOCAL_NAME),Type File Depends DependLibs,proj_print,DEPEND_ITEMS_TEMP,DEPEND_OUTPUT)

DEPEND_FILES		:= $(shell echo "$(DEPEND_OUTPUT)" | awk -f $(ROOT_MAKE_DIRECTORY)/awk/depend.awk)

SYSTEM_LIB 		:= $(filter-out /%,$(DEPEND_FILES))
DEPEND_FILES 		:= $(filter /%,$(DEPEND_FILES))
DEPEND_FILES  		:= $(addprefix $(PLATFORM_OUTPUT_DIRECTORY),$(DEPEND_FILES))

DEPEND_PATHS		:= $(dir $(DEPEND_FILES))
DEPEND_PATHS		:= $(call uniq,$(DEPEND_PATHS))
DEPEND_NAMES		:= $(notdir$(DEPEND_FILES))
