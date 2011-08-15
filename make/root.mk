################################################################################
## @file: 	root.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	从根目录开始的make.
## @version	1.0
###############################################################################

include $(RELETIVE_DIR)/rootdir.mk

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

PLATFORM_NAME_ALL	:= $(notdir $(wildcard $(ROOT_PLATFORM_DIRECTORY)/*))

MAKECMDGOALS_NO_SLASH	:= $(patsubst %/,%,$(MAKECMDGOALS))

PLATFORM_TARGETS	:= $(filter $(PLATFORM_NAME_ALL) all-platform,$(MAKECMDGOALS_NO_SLASH))

OTHER_TARGETS		:= $(filter-out $(PLATFORM_TARGETS),$(MAKECMDGOALS))

PASS_TARGETS		:= $(filter-out $(addsuffix /,$(PLATFORM_TARGETS)),$(OTHER_TARGETS))

ifeq ($(PLATFORM_TARGETS),)
	PLATFORM_TARGETS	:= $(wildcard *)
endif

.PHONY: target
target: $(PLATFORM_TARGETS)

.PHONY: all-platform
all-platform : $(PLATFORM_NAME_ALL)

.PHONY: $(PLATFORM_NAME_ALL)
$(PLATFORM_NAME_ALL) : % : boot//%
	@echo $@
	$(MAKE) -C $@ config="$(strip $(config))" $(PASS_TARGETS)
	@echo

.PHONY: $(OTHER_TARGETS)
$(OTHER_TARGETS) : $(PLATFORM_TARGETS)

include $(ROOT_MAKE_DIRECTORY)/boot.mk

$(foreach platform,$(PLATFORM_NAME_ALL),$(call make_code_boot,/$(platform)))
