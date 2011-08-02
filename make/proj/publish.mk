################################################################################
## @file:	publish.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	发布目标
## @version	1.0
###############################################################################

include $(PROJ_MAKE_DIRECTORY)/version.mk

PUBLISH_MAKE_DIRECTORY		:= $(PROJ_MAKE_DIRECTORY)/publish

ifeq ($(PROJECT_TYPE),lib)
	PUBLISH_DIRECTORY	:= $(ROOT_LIBRARY_DIRECTORY)
else
	PUBLISH_DIRECTORY	:= $(ROOT_BINARY_DIRECTORY)
endif

$(info CONFIG_publish=$(CONFIG_publish))

ifeq ($(findstring test,$(CONFIG_publish)),test)

        include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

	REVERT_TEST_DIRECTORY	:= $(strip $(call revert_directory,$(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)))

endif


.PHONY: publish
publish : $(addprefix publish-,$(CONFIG_publish))

.PHONY: publish-private
publish-private : $(TARGET_FILE_FULL)
	@$(ECHO) "$(MKDIR) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)"
	@$(MKDIR) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)
	@$(ECHO) "$(LN) $(TARGET_FILE_FULL) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) $(TARGET_FILE_FULL) $(PUBLISH_DIRECTORY)$(LOCAL_NAME)/$(TARGET_FILE_VERSION)

.PHONY: publish-public
publish-public : $(TARGET_FILE_FULL)
	@$(ECHO) "$(MKDIR) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)"
	@$(MKDIR) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)	
	@$(ECHO) "$(LN) $(TARGET_FILE_FULL) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) $(TARGET_FILE_FULL) $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)

.PHONY: publish-test
publish-test : $(TARGET_FILE_FULL)
	@$(ECHO) "$(MKDIR) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)"
	@$(MKDIR) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME) 
	@$(ECHO) "$(LN) -s $(REVERT_TEST_DIRECTORY)/$(TARGET_FILE_FULL) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)"
	@$(LN) -s $(REVERT_TEST_DIRECTORY)/$(TARGET_FILE_FULL) $(ROOT_TEST_DIRECTORY)/$(PLATFORM_NAME)/$(TARGET_FILE_VERSION)
