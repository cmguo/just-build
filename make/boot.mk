################################################################################
## @file:	boot.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	自动创建Makefile.
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

define code_boot
boot/$(1) : boot/$(call directory,$(1)) $(call directory,$(ROOT_OUTPUT_DIRECTORY)$(1))/BootStrap.mk

endef

make_code_boot		= $(eval $(call code_boot,$(1))) $(if $(call directory,$(1)),$(call make_code_boot,$(call directory,$(1))))

$(ROOT_OUTPUT_DIRECTORY):
	@$(MKDIR) $(ROOT_OUTPUT_DIRECTORY)
	@test -f $(ROOT_OUTPUT_DIRECTORY)/Makefile || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/outputmakefile.mk $(ROOT_OUTPUT_DIRECTORY)/Makefile
	@test -f $(ROOT_OUTPUT_DIRECTORY)/BootStrap.mk || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/bootstrap.mk $(ROOT_OUTPUT_DIRECTORY)/BootStrap.mk

boot//% : $(ROOT_OUTPUT_DIRECTORY)
	@$(MKDIR) $(patsubst boot//%,$(ROOT_OUTPUT_DIRECTORY)/%,$@)
	@test -f $(ROOT_OUTPUT_DIRECTORY)/$(patsubst boot/%,%,$@)/Makefile || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/makefile.mk $(ROOT_OUTPUT_DIRECTORY)/$(patsubst boot/%,%,$@)/Makefile

%/BootStrap.mk : 
	@test -f $@ || $(CP) $(ROOT_MAKE_DIRECTORY)/tpl/bootstrap.mk $@

boot/ :

