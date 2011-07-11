################################################################################
## @file:	publish.mk
## @author	����ï <gcm.ustc.edu>
## @brief	����Ŀ��
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
