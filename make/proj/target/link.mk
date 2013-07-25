################################################################################
## @file:       link.mk
## @author      张框正
## @brief       生成动态链接库的规则
## @version     1.0
###############################################################################

ifneq ($(CONFIG_LIB),static) # dynamic static2

include $(PROJ_MAKE_DIRECTORY)/depends.mk

LIB_PATHS		:= $(dir $(DEPEND_FILES))
LIB_NAMES		:= $(notdir $(DEPEND_FILES))
LIB_NAMES		:= $(patsubst lib%.a,%,$(LIB_NAMES))
LIB_NAMES		:= $(patsubst lib%$(DYNAMIC_NAME_SUFFIX),%,$(LIB_NAMES))

ifneq ($(CONFIG_LIB),static2) # dynamic

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)
LIB_NAMES		:= $(LIB_NAMES) $(SYSTEM_LIB)
LIB_NAMES		:= $(filter-out $(PLATFORM_DEPEND_LIBRARYS),$(LIB_NAMES))
LIB_NAMES		:= $(LIB_NAMES) $(PLATFORM_DEPEND_LIBRARYS)

ifeq ($(CONFIG_COMPILE),release)
        ifneq ($(PROJECT_VERSION_SCRIPT),)
		VERSION_SCRIPT		:= $(SOURCE_DIRECTORY)/$(PROJECT_VERSION_SCRIPT)
        endif
endif

ifneq ($(VERSION_SCRIPT),)
	LINK_FLAGS              := $(LINK_FLAGS) -Wl,--version-script=$(VERSION_SCRIPT)
	DEPEND_FILES		:= $(VERSION_SCRIPT) $(DEPEND_FILES)
endif

LINK_FLAGS		:= $(LINK_FLAGS) -Wl,-rpath=.

LINK_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(LINK_FLAGS))

endif # ifneq ($(CONFIG_LIB),static2)

LINK_FLAGS		:= $(strip $(LINK_FLAGS))

LIB_PATHS		:= $(addprefix -L,$(LIB_PATHS))
LIB_NAMES		:= $(addprefix -l,$(LIB_NAMES))

endif # ifneq ($(CONFIG_LIB),static)

SOURCE_OBJECTS_FULL	:= $(addprefix $(OBJECT_DIRECTORY)/, $(SOURCE_OBJECTS))

ifneq ($(PLATFORM_CRTBEGIN_STATICBIN),)
	PLATFORM_CRTBEGIN_STATICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_STATICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_STATICBIN),)
	PLATFORM_CRTEND_STATICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_STATICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTBEGIN_DYNAMICBIN),)
	PLATFORM_CRTBEGIN_DYNAMICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_DYNAMICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_DYNAMICBIN),)
	PLATFORM_CRTEND_DYNAMICBIN	:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_DYNAMICBIN),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTBEGIN_DYNAMIC),)
	PLATFORM_CRTBEGIN_DYNAMIC	:= $(wildcard $(addsuffix /$(PLATFORM_CRTBEGIN_DYNAMIC),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif

ifneq ($(PLATFORM_CRTEND_DYNAMIC),)
	PLATFORM_CRTEND_DYNAMIC		:= $(wildcard $(addsuffix /$(PLATFORM_CRTEND_DYNAMIC),$(PLATFORM_LIBRARY_DIRECTORYS)))
endif
