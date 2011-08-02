################################################################################
## @file:	lib.mk
## @author	����ï <gcm.ustc.edu>
## @brief	�������ӿ�Ĺ���
## @version	1.0
###############################################################################

ifeq ($(findstring nopic,$(COMPILE_FLAGS)),nopic)
	COMPILE_FLAGS		:= $(filter-out nopic,$(COMPILE_FLAGS))
else
	COMPILE_FLAGS		:= $(COMPILE_FLAGS) -fPIC
endif

include $(TARGET_MAKE_DIRECTORY)/$(CONFIG_LIB).mk
