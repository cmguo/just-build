################################################################################
## @file: 	depends.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	寻找依赖项.
## @version	1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY)/func/info.mk
include $(ROOT_MAKE_DIRECTORY)/func/pinfo.mk
include $(ROOT_MAKE_DIRECTORY)/func/packinfo.mk

#iiiii := $(call fromat_depends,$(LOCAL_NAME),dirs_z,depends_z)

define proj_print
$(2) := $(strip $($(2)) $(1):$(call get_item,$(1),File):$(call get_item,$(1),Depends):$(call get_item,$(1),DependLibs)!)
endef

iiii := $(call tree_visit,$(LOCAL_NAME),File Depends DependLibs,proj_print,dirs_z,depends_z)


DEPEND_FILES := $(shell echo "$(depends_z)" | awk -f $(ROOT_MAKE_DIRECTORY)/awk/depend.awk)

SYSTEM_LIB := $(filter-out %.a %.so,$(DEPEND_FILES))
DEPEND_FILES := $(filter %.a %.so,$(DEPEND_FILES))
DEPEND_FILES  := $(addprefix $(PLATFORM_DIRECTORY),$(DEPEND_FILES))

