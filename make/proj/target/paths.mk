################################################################################
## @file:	paths.mk
## @author	����ï <gcm.ustc.edu>
## @brief	����Ŀ¼ϸ��
## @version	1.0
###############################################################################

INC_PATHS		:= $(INC_PATHS) $(wildcard $(addsuffix include,$(DEPEND_PATHS)))
INC_PATHS		:= $(INC_PATHS) $(HEADER_DIRECTORYS)

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_PATHS		:= $(LIB_PATHS) $(DEPEND_PATHS)

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)

LIB_PATHS2		:= $(filter %/,$(DEPEND_FILES))
LIB_PATHS2		:= $(call joinlist,:,$(strip $(LIB_PATHS2) $(LD_LIBRARY_PATH)))

endif # dynamic, bin

endif # dynamic static2, bin
