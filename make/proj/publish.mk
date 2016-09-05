################################################################################
## @file:	publish.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	发布目标
## @version	1.0
###############################################################################

PUBLISH_MAKE_DIRECTORY		:= $(PROJ_MAKE_DIRECTORY)/publish

ifeq ($(PROJECT_TYPE),lib)
publish-private: PUBLISH_DIRECTORY 	:= $(ROOT_LIBRARY_DIRECTORY)$(LOCAL_NAME)

        ifeq ($(CONFIG_LIB)-$(DYNAMIC_NAME_SUFFIX),dynamic-.dll)
        TARGET_NAME_FULL := $(TARGET_NAME_FULL:%.dll=%.a)
        TARGET_NAME_FULL := $(TARGET_NAME_FULL:%.dll=%.a)
        endif

else
publish-private: PUBLISH_DIRECTORY 	:= $(ROOT_BINARY_DIRECTORY)$(LOCAL_NAME)
endif


publish-public: PUBLISH_DIRECTORY	:= $(ROOT_PUBLISH_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)

PUBLISH_TEST_DIRECTORY		:= $(ROOT_TEST_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)

publish-test: PUBLISH_DIRECTORY		:= $(PUBLISH_TEST_DIRECTORY)

ifeq ($(findstring test,$(CONFIG_publish)),test)

        include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

REVERT_TEST_DIRECTORY	:= $(strip $(call revert_directory,$(PUBLISH_TEST_DIRECTORY)))

endif

.PHONY: publish
publish : $(addprefix publish-,$(CONFIG_publish))

.PHONY: publish-private publish-public
publish-private publish-public : $(TARGET_PATH_PRIVATE)
	$(MKDIR) $(PUBLISH_DIRECTORY)
	$(LN) $(TARGET_FILE_FULL) $(PUBLISH_DIRECTORY)
ifneq ($(TARGET_NAME_MAJOR),)
	$(LN) -s $(TARGET_NAME_FULL) $(PUBLISH_DIRECTORY)/$(TARGET_NAME_MAJOR)
endif

.PHONY: publish-test
publish-test : $(TARGET_FILE_FULL) $(TARGET_FILE_MAJOR)
	$(MKDIR) $(PUBLISH_DIRECTORY) 
	$(LN) -s $(addprefix $(REVERT_TEST_DIRECTORY)/,$(TARGET_FILE_FULL) $(TARGET_FILE_MAJOR)) $(PUBLISH_DIRECTORY)
