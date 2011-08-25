################################################################################
## @file: 	bin.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成可执行文件的规则
## @version	1.0
###############################################################################

include $(TARGET_MAKE_DIRECTORY)/link.mk

LINK_FLAGS		:= $(LINK_FLAGS)

ifeq ($(CONFIG_COMPILE),release)
        ifneq ($(PROJECT_VERSION_SCRIPT),)
		VERSION_SCRIPT		:= $(SOURCE_DIRECTORY)/$(PROJECT_VERSION_SCRIPT)
        endif
endif

ifneq ($(VERSION_SCRIPT),)
	LINK_FLAGS              := $(LINK_FLAGS) -Wl,--version-script=$(VERSION_SCRIPT)
	DEPEND_FILES		:= $(VERSION_SCRIPT) $(DEPEND_FILES)
endif

LINK_FLAGS		:= $(strip $(LINK_FLAGS))

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(DEPEND_FILES) $(MAKEFILE_LIST)
	@$(RM) $@
	$(LD) $(LINK_FLAGS) $(PLATFORM_BEGIN_OBJECTS) $(addprefix $(OBJECT_DIRECTORY)/, $(SOURCE_OBJECTS)) $(LIB_PATHS) $(LIB_NAMES) $(PLATFORM_END_OBJECTS) -o $@
