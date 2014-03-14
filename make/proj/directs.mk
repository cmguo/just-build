################################################################################
## @file:	directs.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	工程目录
## @version	1.0
###############################################################################

SOURCE_DIRECTORY	:= $(PROJECT_SOURCE_DIRECTORY)

ifeq ($(SOURCE_DIRECTORY),)
	SOURCE_DIRECTORY	:= $(LOCAL_NAME)
endif

SOURCE_DIRECTORY	:= $(patsubst /%,$(ROOT_SOURCE_DIRECTORY)/%,$(SOURCE_DIRECTORY))

BUILDABLE		:= $(if $(wildcard $(SOURCE_DIRECTORY)),yes,no)

HEADER_DIRECTORY	:= $(PROJECT_HEADER_DIRECTORY)

ifeq ($(HEADER_DIRECTORY),)
	HEADER_DIRECTORY	:= $(LOCAL_NAME)
endif

HEADER_DIRECTORY	:= $(patsubst /%,$(ROOT_HEADER_DIRECTORY)/%,$(HEADER_DIRECTORY))

HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(SOURCE_DIRECTORY)
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(addprefix $(ROOT_SOURCE_DIRECTORY)/,$(PROJECT_INTERNAL_INCLUDES))
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(ROOT_SOURCE_DIRECTORY)
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(HEADER_DIRECTORY)
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(addprefix $(ROOT_HEADER_DIRECTORY)/,$(PROJECT_EXTERNAL_INCLUDES))
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(ROOT_HEADER_DIRECTORY)
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(PLATFORM_INCLUDE_DIRECTORYS)
HEADER_DIRECTORYS	:= $(HEADER_DIRECTORYS) $(addprefix $(ROOT_HEADER_DIRECTORY)/,$(GLOBAL_INCLUDE_DIRECTORYS))
HEADER_DIRECTORYS	:= $(wildcard $(HEADER_DIRECTORYS))

TARGET_DIRECTORY	:= $(TARGET_DIRECTORY)/$(CONFIG_THREAD)

DEPEND_DIRECTORY	:= $(TARGET_DIRECTORY)/.deps

OBJECT_DIRECTORY	:= $(TARGET_DIRECTORY)/.objs
