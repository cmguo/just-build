################################################################################
## @file: 	proj.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	管理工程的规则文件
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/cmd.mk
include $(ROOT_MAKE_DIRECTORY)/config.mk

ROOT_MAKE_DIRECTORY_PROJ	:= $(ROOT_MAKE_DIRECTORY)/proj

include $(ROOT_MAKE_DIRECTORY_PROJ)/config.mk

include $(ROOT_MAKE_DIRECTORY_PROJ)/directs.mk

TARGETS			:= target slink clean distclean info publish

ifeq ($(MAKECMDGOALS),)
	MAKECMDGOALS		:= target
endif

ifeq ($(BUILDABLE),no)
	MAKECMDGOALS		:= $(subst target,slink,$(MAKECMDGOALS))
endif
include $(patsubst %,$(ROOT_MAKE_DIRECTORY_PROJ)/%.mk,$(filter $(TARGETS),$(MAKECMDGOALS)))
