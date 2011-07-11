################################################################################                                                    
## @file:   public.mk
## @author  张框正
## @brief   处理type=public publish情况下的逻辑处理                                                                                
## @version 1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY)/func/pubinfo.mk

.PHONY: publish
publish: $(TARGET_FILE_FULL)
ifeq ($(CONFIG_publish),yes)
ifneq ($(PROJECT_TYPE)-$(CONFIG_LIB),lib-static)
	@$(ECHO) "$(MKDIR) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)"
	@$(MKDIR) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)	
	@$(ECHO) "$(LN) $(TARGET_FILE_FULL) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) $(TARGET_FILE_FULL) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)
endif
endif


