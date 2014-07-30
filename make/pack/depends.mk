################################################################################
## @file: 	depends.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	寻找依赖项.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/depends.mk

print_proj			= $1:$2:$3:$4:!

DEPEND_OUTPUT		= $(call depend_visit,$(LOCAL_NAME),print_proj,Type File Depends)

DEPEND_FILES		:= $(shell echo "$(DEPEND_OUTPUT)" | awk -vpack=1 -f $(ROOT_MAKE_DIRECTORY)/awk/depend.awk)

DEPEND_FILES 		:= $(filter /%,$(DEPEND_FILES))
