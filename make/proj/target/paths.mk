################################################################################
## @file:	paths.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	处理目录细节
## @version	1.0
###############################################################################

INC_PATHS		:= $(INC_PATHS) $(wildcard $(addsuffix include,$(DEPEND_PATHS)))
INC_PATHS		:= $(INC_PATHS) $(HEADER_DIRECTORYS)

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_PATHS		:= $(LIB_PATHS) $(DEPEND_PATHS)

LIB_NAMES		:= $(notdir $(DEPEND_FILES))
LIB_NAMES		:= $(patsubst lib%.a,%,$(LIB_NAMES))
LIB_NAMES		:= $(patsubst lib%$(DYNAMIC_NAME_SUFFIX),%,$(LIB_NAMES))

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)

LIB_PATHS2		:= $(filter %/,$(DEPEND_FILES))
LIB_PATHS2		:= $(call joinlist,:,$(strip $(LIB_PATHS2) $(LD_LIBRARY_PATH)))

LIB_NAMES		:= $(LIB_NAMES) $(SYSTEM_LIB)
LIB_NAMES		:= $(filter-out $(PLATFORM_DEPEND_LIBRARYS),$(LIB_NAMES))
LIB_NAMES		:= $(LIB_NAMES) $(PLATFORM_DEPEND_LIBRARYS)

endif # dynamic, bin

endif # dynamic static2, bin
