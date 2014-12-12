################################################################################
## @file: 	bin.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成可执行文件的规则
## @version	1.0
###############################################################################

ifneq ($(PLATFORM_CRTBEGIN_DYNAMICBIN),)
	PLATFORM_CRTBEGIN_DYNAMICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_DYNAMICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_DYNAMICBIN),)
	PLATFORM_CRTEND_DYNAMICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_DYNAMICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(DEPEND_FILES) $(MAKEFILE_LIST)
	@echo LD: $@
	@$(RM) $@
	@LD_LIBRARY_PATH=$(LIB_PATHS2) $(LD) $(LINK_FLAGS) $(PLATFORM_CRTBEGIN_DYNAMICBIN) $(SOURCE_OBJECTS_FULL) $(LINK_LIB_PATHS) $(LINK_LIB_NAMES) $(PLATFORM_CRTEND_DYNAMICBIN) -o $@
