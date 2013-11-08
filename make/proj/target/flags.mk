################################################################################
## @file:	flags.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定编译参数.
## @version	1.0
###############################################################################

ifeq ($(DEFAULT_FLAGS),)
	DEFAULT_FLAGS		:= 1
endif

ifeq ($(DEFAULT_FLAGS),1)
        CONFIG_COMPILE_FLAGS			:= -fPIC $(CONFIG_COMPILE_FLAGS)

        ifeq ($(CONFIG_THREAD),multi)
		CONFIG_COMPILE_FLAGS		:= -pthread $(CONFIG_COMPILE_FLAGS)
		CONFIG_LINK_FLAGS		:= -pthread $(CONFIG_LINK_FLAGS)
        endif

        ifeq ($(CONFIG_COMPILE),debug)
		CONFIG_COMPILE_FLAGS		:= -Wall -g -DDEBUG $(CONFIG_COMPILE_FLAGS)
		CONFIG_LINK_FLAGS		:= $(CONFIG_LINK_FLAGS)
        else
		CONFIG_COMPILE_FLAGS		:= -Wall -g -Os -DNDEBUG $(CONFIG_COMPILE_FLAGS)
		CONFIG_LINK_FLAGS		:= -Wl,-Os $(CONFIG_LINK_FLAGS)
        endif

        ifneq ($(findstring static,$(CONFIG_LIB2)),)
                CONFIG_LINK_FLAGS		:=
        endif

        ifeq ($(CONFIG_LIB2),static)
                CONFIG_LINK_FLAGS		:= rcs $(CONFIG_LINK_FLAGS)
        endif
endif

ifneq ($(PLATFORM_SYS_ROOT),)
	SYS_ROOT		:= --sysroot=$(call reletive_directory,$(PLATFORM_SYS_ROOT))
endif

COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(GLOBAL_COMPILE_FLAGS) $(SYS_ROOT) $(PLATFORM_COMPILE_FLAGS) $(PROJECT_COMPILE_FLAGS)

ifeq ($(findstring static,$(CONFIG_LIB2)),)
LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(GLOBAL_LINK_FLAGS) $(SYS_ROOT) $(PLATFORM_LINK_FLAGS) $(PROJECT_LINK_FLAGS)
else
LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(GLOBAL_STATIC_LINK_FLAGS) $(PLATFORM_STATIC_LINK_FLAGS) $(PROJECT_LINK_FLAGS)
endif

COMPILE_INC_PATHS	:= $(addprefix -I,$(INC_PATHS))

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

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

endif # dynamic, bin

LINK_FLAGS		:= $(strip $(LINK_FLAGS))

LINK_LIB_PATHS		:= $(addprefix -L,$(LIB_PATHS))
LINK_LIB_NAMES		:= $(addprefix -l,$(LIB_NAMES))

endif # dynamic static2, bin

COMPILE_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(COMPILE_FLAGS))

LINK_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(LINK_FLAGS))

COMPILE_FLAGS		:= $(strip $(COMPILE_FLAGS))

LINK_FLAGS		:= $(strip $(LINK_FLAGS))
