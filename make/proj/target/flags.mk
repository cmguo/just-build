################################################################################
## @file:	flags.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定编译参数.
## @version	1.0
###############################################################################

ARCHIVE_FLAGS		:= rcs

ifeq ($(DEFAULT_FLAGS),)
	DEFAULT_FLAGS		:= 1
endif

ifeq ($(DEFAULT_FLAGS),1)
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

endif

ifneq ($(PLATFORM_SYS_ROOT),)
	SYS_ROOT		:= --sysroot=$(PLATFORM_SYS_ROOT)
endif

COMPILE_FLAGS		:= -fPIC $(CONFIG_COMPILE_FLAGS) $(GLOBAL_COMPILE_FLAGS) $(SYS_ROOT) $(PLATFORM_COMPILE_FLAGS) $(PROJECT_COMPILE_FLAGS)

LINK_FLAGS		:= $(CONFIG_LINK_FLAGS) $(GLOBAL_LINK_FLAGS) $(SYS_ROOT) $(PLATFORM_LINK_FLAGS) $(PROJECT_LINK_FLAGS)

ARCHIVE_FLAGS		:= $(ARCHIVE_FLAGS) $(CONFIG_STATIC_LINK_FLAGS) $(GLOBAL_STATIC_LINK_FLAGS) $(PLATFORM_STATIC_LINK_FLAGS) $(PROJECT_LINK_FLAGS)

COMPILE_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(COMPILE_FLAGS))

LINK_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(LINK_FLAGS))

ARCHIVE_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(ARCHIVE_FLAGS))

COMPILE_FLAGS		:= $(strip $(COMPILE_FLAGS))

LINK_FLAGS		:= $(strip $(LINK_FLAGS))

ARCHIVE_FLAGS		:= $(strip $(ARCHIVE_FLAGS))
