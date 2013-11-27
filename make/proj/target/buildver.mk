################################################################################
## @file:	buildver.mk
## @author	����ï <gcm.ustc.edu>
## @brief	�޸�build�汾��
## @version	1.0
###############################################################################

ifeq ($(BUILDABLE)$(CONFIG_build_version),yesyes)

MAKE_VERSION_FILE	:= $(TARGET_DIRECTORY)/file.version

target: $(MAKE_VERSION_FILE)

$(MAKE_VERSION_FILE): $(TARGET_FILE_FULL) $(BUILD_VERSION_FILE)
	@touch -r $(TARGET_FILE_FULL) $@
	$(EV) $(TARGET_FILE_FULL) $(VERSION_NAME) $(VERSION).$(BUILD_VERSION)
	@touch -r $@ $(TARGET_FILE_FULL)
	@echo $(VERSION).$(BUILD_VERSION) > $@

endif
