################################################################################
## @file: 	static.mk
## @author	����ï <gcm.ustc.edu>
## @brief	���ɾ�̬���ӿ�Ĺ���
## @version	1.0
###############################################################################

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST)
	@$(RM) $@
	$(AR) $(LINK_FLAGS) $@ $(SOURCE_OBJECTS_FULL)
