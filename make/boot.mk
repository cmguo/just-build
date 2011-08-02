################################################################################
## @file:	boot.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	自动创建Makefile.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

define code_boot
boot/$(1) : boot/$(call directory,$(1)) $(call directory,$(ROOT_BUILD_DIRECTORY)$(1))/BootStrap.mk

endef

make_code_boot		= $(eval $(call code_boot,$(1))) $(if $(call directory,$(1)),$(call make_code_boot,$(call directory,$(1))))

boot//% : 
	@$(MKDIR) $(patsubst boot//%,$(ROOT_BUILD_DIRECTORY)/%,$@)
	@ls $(ROOT_BUILD_DIRECTORY)/$(patsubst boot/%,%,$@)/Makefile || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/makefile.mk $(ROOT_BUILD_DIRECTORY)/$(patsubst boot/%,%,$@)/Makefile

%/BootStrap.mk : 
	@ls $@ || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/bootstrap.mk $@

boot/ :

