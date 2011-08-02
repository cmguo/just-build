################################################################################
## @file:       lin.mk
## @author      张框正
## @brief       生成动态链接库的规则
## @version     1.0
###############################################################################
include $(PROJ_MAKE_DIRECTORY)/depends.mk

LIB_PATHS               := $(addprefix -L,$(dir $(DEPEND_FILES)))
LIB_NAMES               := $(addprefix -l,$(patsubst lib%.so,%,$(patsubst lib%.a,%,$(notdir  $(DEPEND_FILES)))))
LIB_NAMES               := $(LIB_NAMES) $(addprefix -l,$(SYSTEM_LIB))
LIB_NAMES               := $(LIB_NAMES) $(addprefix -l,$(PLATFORM_DEPEND_LIBRARYS))
