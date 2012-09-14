################################################################################
## @file:       target.mk
## @author      张框正
## @brief       删除临时文件，中间文件
## @version     1.0
###############################################################################

MAKE_DIRECTORYS         := $(TARGET_DIRECTORY) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)

ifneq ($(CONFIG_packet),)
include $(ROOT_STRATEGY_DIRECTORY)/packet/$(CONFIG_packet).mk
endif

call_post_action	= $(foreach action,$(2),$(call $(action),$(1)) && )true

.PHONY: target
target: $(TARGET_FILE_FULL)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

.PHONY: $(PACKET_DEPEND_FILES)
$(PACKET_DEPEND_FILES): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	@svn up $(ROOT_DIRECTORY)$@ 2>&0 > /dev/null
	@svn export $(ROOT_DIRECTORY)$@ $(TARGET_DIRECTORY)/$(notdir $@) 2>&0 > /dev/null

.PHONY: $(PACKET_DEPEND_FILES2)
$(PACKET_DEPEND_FILES2): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	$(CP) -r $(PLATFORM_BUILD_DIRECTORY)$@ $(TARGET_DIRECTORY)/$(notdir $@) 2>&0 > /dev/null

define pack_depend
$(call pack_depend2,$(1),$(call get_item_type,$(1)),$(call get_item_file,$(1)))
endef

# pack_depend2 name type  file
define pack_depend2
$(if $(findstring static,$(2)), \
    $(CP) $(PLATFORM_BUILD_DIRECTORY)$(1)/$(3) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3)), \
    $(STRIP) $(PLATFORM_BUILD_DIRECTORY)$(1)/$(3) -o $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3))) && \
$(call call_post_action,$(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3)),$(PACKET_POST_ACTION))
endef

.PHONY: $(PACKET_DEPENDS)
$(PACKET_DEPENDS): mkdirs
	@$(ECHO) $@
	$(call pack_depend,$@)

$(info TARGET_FILE_2=$(TARGET_FILE_2))
$(TARGET_FILE_FULL): $(PACKET_DEPENDS) $(PACKET_DEPEND_FILES) $(PACKET_DEPEND_FILES2) $(MAKEFILE_LIST) 
	$(CD) $(TARGET_DIRECTORY) ; tar -czv -f $(TARGET_FILE_2) $(PLATFORM_STRATEGY_NAME) $(notdir $(PACKET_DEPEND_FILES)) $(notdir $(PACKET_DEPEND_FILES2)) 2>&0 > /dev/null
