################################################################################
## @file: 	static.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	生成静态链接库的规则
## @version	1.0
###############################################################################

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST)
	@$(RM) $@
	$(AR) $(LINK_FLAGS) $@ $(SOURCE_OBJECTS_FULL)
