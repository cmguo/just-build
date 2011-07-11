################################################################################                                                    
## @file:   test.mk
## @author  张框正
## @brief   处理type=test publish情况下的逻辑处理                                                                                
## @version 1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/pubinfo.mk

REVERT_TEST_DIRECTORY   := $(strip $(call revert_directory,$(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)))

.PHONY: publish
publish: $(TARGET_FILE_FULL)
ifeq ($(CONFIG_publish),yes)
ifneq ($(PROJECT_TYPE)-$(CONFIG_LIB),lib-static)
	@$(ECHO) "$(MKDIR) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)"
	@$(MKDIR) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME) 
	@$(ECHO) "$(LN) -s $(REVERT_TEST_DIRECTORY)/$(TARGET_FILE_FULL) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) -s $(REVERT_TEST_DIRECTORY)/$(TARGET_FILE_FULL) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)
endif
endif


