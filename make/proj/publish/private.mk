################################################################################                                                    
## @file:   private.mk
## @author  张框正
## @brief   处理type=private publish情况下的逻辑处理                                                                               
## @version 1.0
###############################################################################
include $(ROOT_MAKE_DIRECTORY)/func/pubinfo.mk

$(info $(PROJECT_TYPE))
.PHONY: publish
publish: $(TARGET_FILE_FULL)
ifeq ($(CONFIG_publish),yes)
ifneq ($(PROJECT_TYPE)-$(CONFIG_LIB),lib-static)
	@$(ECHO) "$(MKDIR) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)"
	@$(MKDIR) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)
	@$(ECHO) "$(LN) $(TARGET_FILE_FULL) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) $(TARGET_FILE_FULL) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)/$(TARGET_FILE_VERSION)
endif
endif


