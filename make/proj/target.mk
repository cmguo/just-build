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

include $(TARGET_MAKE_DIRECTORY)/flags.mk

ifneq ($(PROJECT_TYPE),auto)

include $(TARGET_MAKE_DIRECTORY)/files.mk

include $(TARGET_MAKE_DIRECTORY)/compile.mk

endif

MAKE_DIRECTORYS		:= $(DEPEND_DIRECTORY) $(OBJECT_DIRECTORY) $(DEPEND_SUB_DIRECTORYS) $(OBJECT_SUB_DIRECTORYS)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

include $(PROJ_MAKE_DIRECTORY)/shell.mk

DEPENDS                 := $(PROJECT_DEPENDS) $(PLATFORM_DEPENDS)

include $(TARGET_MAKE_DIRECTORY)/$(PROJECT_TYPE).mk

target: $(TARGET_FILE_FULL)

ifeq ($(BUILDABLE)$(CONFIG_build_version),yesyes)

MAKE_VERSION_FILE	:= $(TARGET_FILE_FULL).version

target: $(MAKE_VERSION_FILE)

$(MAKE_VERSION_FILE): $(TARGET_FILE_FULL) $(BUILD_VERSION_FILE)
	$(EV) $(TARGET_FILE_FULL) $(VERSION_NAME) $(VERSION).$(BUILD_VERSION)
	@touch $@

endif
