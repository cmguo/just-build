################################################################################
## @file:	buildver.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	修改build版本号
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
