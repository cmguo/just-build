################################################################################
## @file:   target.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief   生成发布包的规则
## @version 1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/repo.mk
include $(PACK_MAKE_DIRECTORY)/depends.mk

TARGET_STRIP_DIRECTORY	:= $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)
TARGET_SYMBOL_DIRECTORY	:= $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)-symbol
MAKE_DIRECTORYS         := $(TARGET_DIRECTORY) $(TARGET_STRIP_DIRECTORY) $(TARGET_SYMBOL_DIRECTORY)

DEPEND_FILES1			:= $(PACKET_DEPEND_FILES:%=$(ROOT_DIRECTORY)%)
DEPEND_FILES2			:= $(PACKET_DEPEND_FILES2:%=$(PLATFORM_OUTPUT_DIRECTORY)%)

BUILD_PROP_FILE			:= $(TARGET_DIRECTORY)/build.prop

$(shell echo version.product=$(VERSION) > $(BUILD_PROP_FILE))
$(shell echo version.type=$(VERSION_TYPE) >> $(BUILD_PROP_FILE))
$(shell echo version.branch=$(BRANCH) >> $(BUILD_PROP_FILE))
$(shell echo version.build=$(BUILD_NUMBER) >> $(BUILD_PROP_FILE))
$(shell echo version.time=$(shell date +%s) >> $(BUILD_PROP_FILE))
$(shell echo version.name=$(VERSION_ALL) >> $(BUILD_PROP_FILE))

ifneq ($(CONFIG_packet),)
include $(ROOT_STRATEGY_DIRECTORY)/packet/$(CONFIG_packet).mk
endif

call_post_action	= $(foreach action,$(2),$(call $(action),$(1)) && )true

.PHONY: target
target: $(TARGET_FILE_FULL)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

.PHONY: $(DEPEND_FILES1)
$(DEPEND_FILES1): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	@$(call repo_export,$@,$(TARGET_DIRECTORY)) > /dev/null

.PHONY: $(DEPEND_FILES2)
$(DEPEND_FILES2): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	$(CP) -r $@ $(TARGET_DIRECTORY)/$(notdir $@) > /dev/null

shell_escape2		= $(if $(2),$(call shell_escape2,$(subst $(firstword $(2)),\$(firstword $(2)),$(1)),$(wordlist 2,$(words $(2)),$(2))),$(1))

shell_escape_char	:= ( )

shell_escape		= $(call shell_escape2,$(1),$(shell_escape_char))

define make_depend_rule3
$1 : PRIVATE_LINKS := $(notdir $2)
DEPEND_FILES3	+= $1
.PHONY: $1
$1 : mkdirs
	@$(ECHO) $1
	$(if $2,$(LN) -s $$(notdir $1) $$(PRIVATE_LINKS:%=$(TARGET_STRIP_DIRECTORY)/%))
	$(if $(findstring /static/,$(1)), \
		$(CP) $(PLATFORM_OUTPUT_DIRECTORY)$(1) $(TARGET_STRIP_DIRECTORY)/$(notdir $(1)), \
		$(STRIP) $(PLATFORM_OUTPUT_DIRECTORY)$(1) -o $(TARGET_STRIP_DIRECTORY)/$(notdir $(1)); \
		dump_syms $(PLATFORM_OUTPUT_DIRECTORY)$(1) $(TARGET_SYMBOL_DIRECTORY) > $(TARGET_SYMBOL_DIRECTORY)/$(notdir $(1)).sym; \
	    $$(foreach l,$$(PRIVATE_LINKS),dump_syms $(PLATFORM_OUTPUT_DIRECTORY)/$(dir $(1))$$(l) $(TARGET_SYMBOL_DIRECTORY) > $(TARGET_SYMBOL_DIRECTORY)/$$(l).sym;))
	$(call call_post_action,$(TARGET_STRIP_DIRECTORY)/$(notdir $(1)),$(PACKET_POST_ACTION))
endef

make_depend_rule2 = $(call make_depend_rule3,$(firstword $1),$(wordlist 2,$(words $1),$1))
make_depend_rule = $(eval $(call make_depend_rule2,$(subst :, ,$(1))))

$(foreach d,$(DEPEND_FILES),$(call make_depend_rule,${d}))

$(TARGET_FILE_FULL): $(DEPEND_FILES3) $(DEPEND_FILES1) $(DEPEND_FILES2) $(MAKEFILE_LIST)
	$(CD) $(TARGET_DIRECTORY) ; $(PACK) $(call shell_escape,$(TARGET_FILE)) $(PLATFORM_STRATEGY_NAME) $(PLATFORM_STRATEGY_NAME)-symbol $(notdir $(PACKET_DEPEND_FILES) $(PACKET_DEPEND_FILES2))
	$(RM) $(TARGET_STRIP_DIRECTORY) $(TARGET_SYMBOL_DIRECTORY)
