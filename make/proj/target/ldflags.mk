################################################################################
## @file:	ldflags.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定链接参数.
## @version	1.0
###############################################################################

LINK_FLAGS_all			?=

LINK_FLAGS_debug		?=
LINK_FLAGS_release		?=
LINK_FLAGS_multi		?= -pthread
LINK_FLAGS_single		?=

LINK_FLAGS_debug_multi		?=
LINK_FLAGS_debug_single		?=
LINK_FLAGS_release_multi	?=
LINK_FLAGS_release_single	?=

CONFIG_LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(LINK_FLAGS_all)
CONFIG_LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(LINK_FLAGS_$(CONFIG_PROFILE))
CONFIG_LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(LINK_FLAGS_$(CONFIG_THREAD))
CONFIG_LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(LINK_FLAGS_$(CONFIG_PROFILE)_$(CONFIG_THREAD))

ifneq ($(findstring static,$(CONFIG_LIB2)),)
        CONFIG_LINK_FLAGS		:=
endif

ifeq ($(CONFIG_LIB2),static)
        CONFIG_LINK_FLAGS		:= rcs $(CONFIG_LINK_FLAGS)
endif

ifeq ($(findstring static,$(CONFIG_LIB2)),)
LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(GLOBAL_LINK_FLAGS) $(SYS_ROOT) $(PLATFORM_LINK_FLAGS) $(PROJECT_LINK_FLAGS)
else
LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(GLOBAL_STATIC_LINK_FLAGS) $(PLATFORM_STATIC_LINK_FLAGS) $(PROJECT_LINK_FLAGS)
endif

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_NAMES		:= $(notdir $(DEPEND_FILES))
LIB_NAMES		:= $(LIB_NAMES:$(DYNAMIC_NAME_PREFIX)%.dll=$(STATIC_NAME_PREFIX)%.dll.a)
LIB_NAMES		:= $(LIB_NAMES:$(DYNAMIC_NAME_PREFIX)%$(DYNAMIC_NAME_SUFFIX)=%)
LIB_NAMES		:= $(LIB_NAMES:$(STATIC_NAME_PREFIX)%$(STATIC_NAME_SUFFIX)=%)

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_NAMES		:= $(LIB_NAMES) $(SYSTEM_LIB)
LIB_NAMES		:= $(filter-out $(PLATFORM_DEPEND_LIBRARYS),$(LIB_NAMES))
LIB_NAMES		:= $(LIB_NAMES) $(PLATFORM_DEPEND_LIBRARYS)

ifeq ($(CONFIG_PROFILE),release)
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

endif # dynamic, bin

LINK_FLAGS		:= $(strip $(LINK_FLAGS))

LINK_LIB_PATHS		:= $(addprefix -L,$(LIB_PATHS))
LINK_LIB_NAMES		:= $(addprefix -l,$(LIB_NAMES))
LINK_LIB_NAMES		:= $(filter-out $(PLATFORM_DISABLE_FLAGS),$(LINK_LIB_NAMES))

endif # dynamic static2, bin

LINK_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(LINK_FLAGS))

LINK_FLAGS		:= $(strip $(LINK_FLAGS))
