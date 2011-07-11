################################################################################
## @file: 	target.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成工程目标
## @version	1.0
###############################################################################

.PHONY: target
target: | mkdirs

ROOT_MAKE_DIRECTORY_TARGET	:= $(ROOT_MAKE_DIRECTORY_PROJ)/target

include $(ROOT_MAKE_DIRECTORY_TARGET)/files.mk

include $(ROOT_MAKE_DIRECTORY_TARGET)/flags.mk

include $(ROOT_MAKE_DIRECTORY_TARGET)/compile.mk

MAKE_DIRECTORYS		:= $(DEPEND_DIRECTORY) $(OBJECT_DIRECTORY) $(DEPEND_SUB_DIRECTORYS) $(OBJECT_SUB_DIRECTORYS)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

include $(ROOT_MAKE_DIRECTORY_PROJ)/version.mk

DEPENDS                 := $(PROJECT_DEPENDS) $(PLATFORM_DEPENDS)

#include $(ROOT_MAKE_DIRECTORY_TARGET)/link.mk


include $(ROOT_MAKE_DIRECTORY_TARGET)/$(PROJECT_TYPE).mk
target: $(TARGET_FILE_FULL)
