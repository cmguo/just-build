################################################################################
## @file: 	target.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成工程目标
## @version	1.0
###############################################################################

.PHONY: target
target: | mkdirs

TARGET_MAKE_DIRECTORY	:= $(PROJ_MAKE_DIRECTORY)/target

include $(PROJ_MAKE_DIRECTORY)/depends.mk

CONFIG_LIB2			:= $(CONFIG_LIB)

ifeq ($(PROJECT_TYPE),auto)
	CONFIG_LIB2			:= dynamic
endif

include $(TARGET_MAKE_DIRECTORY)/paths.mk

include $(TARGET_MAKE_DIRECTORY)/ccflags.mk

ifneq ($(PROJECT_TYPE),auto)

include $(TARGET_MAKE_DIRECTORY)/files.mk

include $(TARGET_MAKE_DIRECTORY)/compile.mk

endif

MAKE_DIRECTORYS		:= $(DEPEND_DIRECTORY) $(OBJECT_DIRECTORY) $(DEPEND_SUB_DIRECTORYS) $(OBJECT_SUB_DIRECTORYS)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

include $(TARGET_MAKE_DIRECTORY)/actions.mk

include $(TARGET_MAKE_DIRECTORY)/buildver.mk

include $(TARGET_MAKE_DIRECTORY)/ldflags.mk

include $(TARGET_MAKE_DIRECTORY)/$(PROJECT_TYPE).mk

ifneq ($(NAME_VERSION_MAJOR),)
$(TARGET_FILE_MAJOR) : $(TARGET_FILE_FULL)
	$(LN) -s $(subst $(NAME_VERSION_MAJOR),$(NAME_VERSION),$(notdir $@)) $@
endif

target: $(TARGET_FILE_FULL) $(TARGET_FILE_MAJOR) post_build
