################################################################################
## @file:	files.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	目标文件.
## @version	1.0
###############################################################################

NAME_SUFFIX_SIMPLE	:= -$(PLATFORM_NAME)$(NAME_SUFFIX)

NAME_SUFFIX_CONFIG	:= -$(PLATFORM_NAME)$(NAME_CONFIG)$(NAME_SUFFIX)

NAME_SUFFIX_FULL	:= -$(PLATFORM_NAME)-$(PLATFORM_TOOL_NAME)$(NAME_CONFIG)$(NAME_VERSION)$(NAME_SUFFIX)

ifeq ($(PROJECT_TARGET),)
	PROJECT_TARGET		:= $(PROJECT_TARGETS)
endif

TARGET_FILE		:= $(addprefix $(NAME_PREFIX),$(addsuffix $(NAME_SUFFIX_FULL),$(PROJECT_TARGET)))

TARGET_FILE_FULL	:= $(addprefix $(TARGET_DIRECTORY)/,$(TARGET_FILE))
