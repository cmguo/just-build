################################################################################
## @file: 	static2.mk
## @author	����ï <gcm.ustc.edu>
## @brief	���ɺϲ��ľ�̬���ӿ�Ĺ���
## @version	1.0
###############################################################################

include $(TARGET_MAKE_DIRECTORY)/link.mk

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST) $(DEPEND_FILES)
	@$(RM) $@
	$(LT) $(LINK_FLAGS) $(SOURCE_OBJECTS_FULL) $(LIB_PATHS) $(LIB_NAMES) -o $@
