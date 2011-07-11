################################################################################
## @file: 	pack.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成发布包的规则
## @version	1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY)/cmd.mk

include $(ROOT_MAKE_DIRECTORY)/func/info.mk

ROOT_MAKE_DIRECTORY_PACK     := $(ROOT_MAKE_DIRECTORY)/pack

include $(ROOT_MAKE_DIRECTORY_PACK)/config.mk


TARGETS                 := target clean distclean info publish

ifeq ($(MAKECMDGOALS),)
        MAKECMDGOALS            := target
endif

include $(patsubst %,$(ROOT_MAKE_DIRECTORY_PACK)/%.mk,$(filter $(TARGETS),$(MAKECMDGOALS)))

