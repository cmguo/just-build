################################################################################
## @file: 	static.mk
## @author	����ï <gcm.ustc.edu>
## @brief	���ɾ�̬���ӿ�Ĺ���
## @version	1.0
###############################################################################

ifeq ($(CONFIG_combine_static_lib),yes)
        include $(TARGET_MAKE_DIRECTORY)/link.mk
endif

$(TARGET_FILE_FULL): $(SOURCE_OBJECTS) $(HEADER_OBJECTS) $(MAKEFILE_LIST)
	@$(RM) $@
	$(AR) $(ARFLAGS) $@ $(addprefix $(OBJECT_DIRECTORY)/, $(SOURCE_OBJECTS)) $(DEPEND_FILES)
