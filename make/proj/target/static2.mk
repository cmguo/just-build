################################################################################
## @file: 	static2.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成合并的静态链接库的规则
## @version	1.0
###############################################################################

ifneq ($(PLATFORM_CRTBEGIN_STATICBIN),)
	PLATFORM_CRTBEGIN_STATICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_STATICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_STATICBIN),)
	PLATFORM_CRTEND_STATICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_STATICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST) $(DEPEND_FILES)
	@echo LT: $@
	@$(RM) $@
	@$(LT) $(LINK_FLAGS) $(SOURCE_OBJECTS_FULL) $(LINK_LIB_PATHS) $(LINK_LIB_NAMES) -o $@
