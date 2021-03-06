################################################################################
## @file:	ccflags.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定编译参数.
## @version	1.0
###############################################################################

COMPILE_FLAGS_all			?= -g -fPIC -Wall

COMPILE_FLAGS_debug			?= -DDEBUG
COMPILE_FLAGS_release		?= -Os -DNDEBUG
COMPILE_FLAGS_multi			?= -pthread
COMPILE_FLAGS_single		?=

COMPILE_FLAGS_debug_multi		?=
COMPILE_FLAGS_debug_single		?=
COMPILE_FLAGS_release_multi		?=
COMPILE_FLAGS_release_single	?=

CONFIG_COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(COMPILE_FLAGS_all)
CONFIG_COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(COMPILE_FLAGS_$(CONFIG_PROFILE))
CONFIG_COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(COMPILE_FLAGS_$(CONFIG_THREAD))
CONFIG_COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(COMPILE_FLAGS_$(CONFIG_PROFILE)_$(CONFIG_THREAD))

CONFIG_COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(CONFIG_MACROS:%=-D%)

ifneq ($(PLATFORM_SYS_ROOT),)
	SYS_ROOT		:= --sysroot=$(call reletive_directory,$(PLATFORM_SYS_ROOT))
endif

COMPILE_FLAGS		:= $(CONFIG_COMPILE_FLAGS) $(GLOBAL_COMPILE_FLAGS) $(SYS_ROOT) $(PLATFORM_COMPILE_FLAGS) $(PROJECT_COMPILE_FLAGS)

COMPILE_INC_PATHS	:= $(addprefix -I,$(INC_PATHS))

COMPILE_FLAGS		:= $(filter-out $(addsuffix %,$(PLATFORM_DISABLE_FLAGS)),$(COMPILE_FLAGS))

COMPILE_FLAGS		:= $(strip $(COMPILE_FLAGS))
