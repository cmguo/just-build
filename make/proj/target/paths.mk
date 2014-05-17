################################################################################
## @file:	paths.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	处理目录细节
## @version	1.0
###############################################################################

INC_PATHS		:= $(INC_PATHS) $(wildcard $(addsuffix include,$(DEPEND_PATHS)))
INC_PATHS		:= $(INC_PATHS) $(HEADER_DIRECTORYS)
INC_PATHS		:= $(INC_PATHS) $(PLATFORM_INCLUDE_DIRECTORYS)
INC_PATHS		:= $(INC_PATHS) $(addprefix $(ROOT_DIRECTORY)/,$(GLOBAL_INCLUDE_DIRECTORYS))

INC_PATHS		:= $(call uniq,$(INC_PATHS))

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_PATHS		:= $(LIB_PATHS) $(DEPEND_PATHS)

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)

LIB_PATHS2		:= $(filter %/,$(DEPEND_FILES))
LIB_PATHS2		:= $(call joinlist,:,$(strip $(LIB_PATHS2) $(LD_LIBRARY_PATH)))

endif # dynamic, bin

endif # dynamic static2, bin

LIB_PATHS		:= $(call uniq,$(LIB_PATHS))
