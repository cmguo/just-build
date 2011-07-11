################################################################################
## @file:       publish.mk
## @author      张框正
## @brief       删除临时文件，中间文件
## @version     1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY_PACK)/target.mk

.PHONY: publish
publish: target
	@$(MKDIR) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)
	@$(LN) $(TARGET_FILE_FULL) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE)
