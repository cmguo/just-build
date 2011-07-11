################################################################################
## @file:	lib.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成链接库的规则
## @version	1.0
###############################################################################

ifeq ($(findstring nopic,$(COMPILE_FLAGS)),nopic)
	COMPILE_FLAGS		:= $(filter-out nopic,$(COMPILE_FLAGS))
else
	COMPILE_FLAGS		:= $(COMPILE_FLAGS) -fPIC
endif

include $(ROOT_MAKE_DIRECTORY_TARGET)/$(CONFIG_LIB).mk
