################################################################################
## @file: 	static.mk
## @author	����ï <gcm.ustc.edu>
## @brief	���ɾ�̬���ӿ�Ĺ���
## @version	1.0
###############################################################################

include $(TARGET_MAKE_DIRECTORY)/link.mk

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST)
	@$(RM) $@
	$(AR) $(ARCHIVE_FLAGS) $@ $(STATIC_LINK_FLAGS) $(SOURCE_OBJECTS_FULL)
