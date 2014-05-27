################################################################################
## @file:   target.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief   生成发布包的规则
## @version 1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/repo.mk

TARGET_STRIP_DIRECTORY	:= $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)
TARGET_SYMBOL_DIRECTORY	:= $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)-symbol
MAKE_DIRECTORYS         := $(TARGET_DIRECTORY) $(TARGET_STRIP_DIRECTORY) $(TARGET_SYMBOL_DIRECTORY)

DEPEND_FILES			:= $(PACKET_DEPEND_FILES:%=$(ROOT_DIRECTORY)%)
DEPEND_FILES2			:= $(PACKET_DEPEND_FILES2:%=$(PLATFORM_OUTPUT_DIRECTORY)%)

ifneq ($(CONFIG_packet),)
include $(ROOT_STRATEGY_DIRECTORY)/packet/$(CONFIG_packet).mk
endif

call_post_action	= $(foreach action,$(2),$(call $(action),$(1)) && )true

.PHONY: target
target: $(TARGET_FILE_FULL)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

.PHONY: $(DEPEND_FILES)
$(DEPEND_FILES): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	@$(call repo_export,$@,$(TARGET_DIRECTORY)) > /dev/null

.PHONY: $(DEPEND_FILES2)
$(DEPEND_FILES2): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	$(CP) -r $@ $(TARGET_DIRECTORY)/$(notdir $@) > /dev/null

define pack_depend
$(call pack_depend2,$(1),$(call get_item_type,$(1)),$(call get_item_file,$(1)))
endef

# pack_depend2 name type  file
define pack_depend2
   	$(CP) $(PLATFORM_OUTPUT_DIRECTORY)$(1)/$(3) $(TARGET_SYMBOL_DIRECTORY)/$(notdir $(3))
	$(if $(findstring dynamic,$(2)), \
		$(STRIP) $(PLATFORM_OUTPUT_DIRECTORY)$(1)/$(3) -o $(TARGET_STRIP_DIRECTORY)/$(notdir $(3)))
	$(call call_post_action,$(TARGET_STRIP_DIRECTORY)/$(notdir $(3)),$(PACKET_POST_ACTION))
endef

shell_escape2		= $(if $(2),$(call shell_escape2,$(subst $(firstword $(2)),\\$(firstword $(2)),$(1)),$(wordlist 2,$(words $(2)),$(2))),$(1))

shell_escape_char	:= ( )

shell_escape		= $(call shell_escape2,$(1),$(shell_escape_char))

.PHONY: $(PACKET_DEPENDS)
$(PACKET_DEPENDS): mkdirs
	@$(ECHO) $@
	$(call pack_depend,$@)

$(TARGET_FILE_FULL): $(PACKET_DEPENDS) $(DEPEND_FILES) $(DEPEND_FILES2) $(MAKEFILE_LIST)
	$(CD) $(TARGET_DIRECTORY) ; tar -czv -f $(call shell_escape,$(TARGET_FILE)) $(PLATFORM_STRATEGY_NAME) $(notdir $(PACKET_DEPEND_FILES) $(PACKET_DEPEND_FILES2))

ifneq ($(CONFIG_symbol),)
$(TARGET_FILE_FULL): $(TARGET_FILE_SYMBOL_FULL)
endif

$(TARGET_FILE_SYMBOL_FULL): $(PACKET_DEPENDS)
	$(CD) $(TARGET_DIRECTORY) ; tar -czv -f $(call shell_escape,$(TARGET_FILE_SYMBOL)) $(PLATFORM_STRATEGY_NAME)-symbol
