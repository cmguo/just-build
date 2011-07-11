################################################################################
## @file: 	root.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	从根目录开始的make.
## @version	1.0
###############################################################################

include $(RELETIVE_DIR)/rootdir.mk

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

PLATFORM_NAME_ALL	:= $(notdir $(wildcard $(ROOT_PLATFORM_DIRECTORY)/*))

PLATFORM_TARGETS	:= $(filter $(PLATFORM_NAME_ALL) all,$(MAKECMDGOALS))

OTHER_TARGETS		:= $(filter-out $(PLATFORM_NAME_ALL) all,$(MAKECMDGOALS))

.PHONY: target
target: $(PLATFORM_TARGETS)

.PHONY: all
all : $(PLATFORM_NAME_ALL)

.PHONY: $(PLATFORM_NAME_ALL)
$(PLATFORM_NAME_ALL) : % : boot//%
	@echo $@
	$(MAKE) -C $@ config="$(strip $(config))" $(OTHER_TARGETS)
	@echo

include $(ROOT_MAKE_DIRECTORY)/boot.mk

$(foreach platform,$(PLATFORM_NAME_ALL),$(call make_code_boot,/$(platform)))
