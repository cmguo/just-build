################################################################################
## @file:	publish.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	发布目标
## @version	1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY_PROJ)/version.mk

ROOT_MAKE_DIRECTORY_PUBLISH   := $(ROOT_MAKE_DIRECTORY_PROJ)/publish

ifeq ($(PROJECT_TYPE),lib)
    PUBLISH_DIRECTORY       := $(ROOT_LIBRARY_DIRECTORY)
else
    PUBLISH_DIRECTORY       := $(ROOT_BINARY_DIRECTORY)
endif

include $(ROOT_MAKE_DIRECTORY_PUBLISH)/$(PublishType).mk
