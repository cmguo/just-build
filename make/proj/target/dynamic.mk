################################################################################
## @file: 	dynamic.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成动态链接库的规则
## @version	1.0
###############################################################################

ifneq ($(PLATFORM_CRTBEGIN_DYNAMIC),)
	PLATFORM_CRTBEGIN_DYNAMIC	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_DYNAMIC),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_DYNAMIC),)
	PLATFORM_CRTEND_DYNAMIC		:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_DYNAMIC),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

LINK_FLAGS		:= $(LINK_FLAGS) -shared -Wl,-soname=$(TARGET_NAME_MAJOR)

ifeq ($(DYNAMIC_NAME_SUFFIX),.dll)
	LINK_FLAGS		:= $(LINK_FLAGS) -Wl,--out-implib,$(TARGET_FILE_FULL:%=%.a)
endif

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(DEPEND_FILES) $(MAKEFILE_LIST)
	@echo LD: $@
	@$(RM) $@
	@LD_LIBRARY_PATH=$(LIB_PATHS2) $(LD) $(LINK_FLAGS) $(PLATFORM_CRTBEGIN_DYNAMIC) $(SOURCE_OBJECTS_FULL) $(LINK_LIB_PATHS) $(LINK_LIB_NAMES) $(PLATFORM_CRTEND_DYNAMIC) -o $@
