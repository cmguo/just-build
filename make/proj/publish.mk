################################################################################
## @file:	publish.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	发布目标
## @version	1.0
###############################################################################

PUBLISH_MAKE_DIRECTORY		:= $(PROJ_MAKE_DIRECTORY)/publish

TARGET_FILE_PRIVATE		:= $(TARGET_FILE)

TARGET_FILE_PUBLIC		:= $(addprefix $(TARGET_DIRECTORY)/,$(TARGET_FILE))

ifeq ($(PROJECT_TYPE),lib)
	PRIVATE_DIRECTORY	:= $(ROOT_LIBRARY_DIRECTORY)$(LOCAL_NAME)

        ifeq ($(CONFIG_LIB)-$(DYNAMIC_NAME_SUFFIX),dynamic-.dll)
		TARGET_FILE_PRIVATE	:= $(TARGET_FILE:%.dll=%.a)
        endif

else
	PRIVATE_DIRECTORY	:= $(ROOT_BINARY_DIRECTORY)$(LOCAL_NAME)
endif


PUBLIC_DIRECTORY	:= $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)

TEST_DIRECTORY		:= $(ROOT_TEST_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)

ifeq ($(findstring test,$(CONFIG_publish)),test)

        include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

	REVERT_TEST_DIRECTORY	:= $(strip $(call revert_directory,$(TEST_DIRECTORY)))

endif

.PHONY: publish
publish : $(addprefix publish-,$(CONFIG_publish))

.PHONY: publish-private
publish-private : $(TARGET_FILE_PRIVATE)
	$(MKDIR) $(PRIVATE_DIRECTORY)
	$(LN) $(TARGET_FILE_PRIVATE) $(PRIVATE_DIRECTORY)

.PHONY: publish-public
publish-public : $(TARGET_FILE_PUBLIC)
	$(MKDIR) $(PUBLIC_DIRECTORY)
	$(LN) $(TARGET_FILE_PUBLIC) $(PUBLIC_DIRECTORY)

.PHONY: publish-test
publish-test : $(TARGET_FILE_PUBLIC)
	$(MKDIR) $(TEST_DIRECTORY) 
	$(LN) -s $(addprefix $(REVERT_TEST_DIRECTORY)/,$(TARGET_FILE_PUBLIC)) $(TEST_DIRECTORY)
