################################################################################
## @file: 	target.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成前、后执行动作
## @version	1.0
###############################################################################

.PHONY: prev_build
prev_build: mkdirs

.PHONY: post_build
post_build: $(TARGET_FILE_FULL)

.PHONY: $(PROJECT_DEPEND_SHELL)
$(PROJECT_DEPEND_SHELL):
	$(SHELL) $(PROJECT_DIRECTORY)/$@

PROJECT_PREV_ACTIONS		:= $(PROJECT_PREV_ACTIONS) $(PROJECT_DEPEND_SHELL)

prev_build: $(PROJECT_PREV_ACTIONS)

post_build: $(PROJECT_POST_ACTIONS)

$(PROJECT_PREV_ACTIONS): $(TARGET_FILE_FULL)

