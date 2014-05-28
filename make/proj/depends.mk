################################################################################
## @file: 	depends.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	寻找依赖项.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/depends.mk

print_proj			= $1:$2:$3:$4:$5!

DEPEND_OUTPUT		= $(call depend_visit,$(LOCAL_NAME),print_proj,Type File Depends DependLibs)

DEPEND_FILES		:= $(shell echo "$(DEPEND_OUTPUT)" | awk -f $(ROOT_MAKE_DIRECTORY)/awk/depend.awk)

SYSTEM_LIB 			:= $(filter-out /%,$(DEPEND_FILES))
DEPEND_FILES 		:= $(filter /%,$(DEPEND_FILES))
DEPEND_FILES  		:= $(addprefix $(PLATFORM_OUTPUT_DIRECTORY),$(DEPEND_FILES))

DEPEND_PATHS		:= $(dir $(DEPEND_FILES))
DEPEND_PATHS		:= $(call uniq,$(DEPEND_PATHS))
DEPEND_NAMES		:= $(notdir$(DEPEND_FILES))
