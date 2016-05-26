################################################################################
## @file:       info.mk
## @author      郭春茂
## @brief       输出打包项目信息
## @version     1.0
###############################################################################

.PHONY: info
info: 
	@$(ECHO) "Name: $(LOCAL_NAME)"
	@$(ECHO) "Type: $(LOCAL_TYPE)"
	@$(ECHO) "Config: $(CONFIG_PROFILE)"
	@$(ECHO) "Directory: $(TARGET_DIRECTORY)"
	@$(ECHO) "Target: $(PACKET_TARGET)"
	@$(ECHO) "Version: $(VERSION)"
	@$(ECHO) "VersionMajor: $(VERSION_MAJOR)"
	@$(ECHO) "File: $(TARGET_FILE_FULL)"
	@$(ECHO) "FileMajor: $(TARGET_FILE_MAJOR)"
	@$(ECHO) "Depends: $(PACKET_DEPENDS)"
