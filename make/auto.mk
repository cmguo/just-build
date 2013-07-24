################################################################################
## @file:	auto.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	管理工程的规则文件
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/cmd.mk
include $(ROOT_MAKE_DIRECTORY)/func/config.mk
include $(ROOT_MAKE_DIRECTORY)/auto/func/info.mk

CONFIG_COMPILE_LIST	:= debug release
CONFIG_COMPILE		:= $(call get_config,$(CONFIG),$(CONFIG_COMPILE_LIST),debug)

CONFIG_THREAD_LIST	:= multi single
CONFIG_THREAD		:= $(call get_config,$(CONFIG),$(CONFIG_THREAD_LIST),multi)

CONFIG_LIB_LIST		:= dynamic static
CONFIG_LIB		:= $(call get_config,$(CONFIG),$(CONFIG_LIB_LIST),static)


PLATFORM_DISABLE_FLAGS	:= $(PLATFORM_DISABLE_FLAGS) -fvisibility

# we need update sysroot to absolute path
PLATFORM_SYS_ROOT	:= $(call reletive_directory,$(PLATFORM_SYS_ROOT))
PLATFORM_INCLUDE_DIRECTORYS	:= $(call reletive_directory,$(PLATFORM_INCLUDE_DIRECTORYS))
PLATFORM_LIBRARY_DIRECTORYS	:= $(call reletive_directory,$(PLATFORM_LIBRARY_DIRECTORYS))

include $(ROOT_MAKE_DIRECTORY)/proj/target/flags.mk

COMPILE_FLAGS		:= $(COMPILE_FLAGS) $(addprefix -I,$(PLATFORM_INCLUDE_DIRECTORYS))

LINK_FLAGS		:= $(LINK_FLAGS) $(addprefix -L,$(PLATFORM_LIBRARY_DIRECTORYS))
LINK_FLAGS		:= $(LINK_FLAGS) $(addprefix -l,$(LOCAL_DEPEND_LIBRARYS))
LINK_FLAGS		:= $(LINK_FLAGS) $(addprefix -l,$(PLATFORM_DEPEND_LIBRARYS))

SOURCE_DIRECTORY	:= $(ROOT_SOURCE_DIRECTORY)$(LOCAL_NAME)

TARGET_DIRECTORY	:= $(CONFIG_COMPILE)/$(CONFIG_LIB)/$(CONFIG_THREAD)
TARGET_FILE_FULL	:= $(TARGET_DIRECTORY)/$(LOCAL_TARGET)

BUILDABLE		:= $(if $(wildcard $(SOURCE_DIRECTORY)),yes,no)

BUILD_TARGET		:= $(PLATFORM_TOOL_TARGET)
ifeq ($(BUILD_TARGET),)
	BUILD_TARGET	:= $(strip $(shell PATH=$(PATH) LANG=C $(PLATFORM_TOOL_PREFIX)gcc -v 2>&1 | awk -F : '$$1 == "Target" { print $$2 }'))
endif

ENVIRONMENT		:= 

CONFIGURE		:=

# Standard options:
CONFIGURE		:= $(CONFIGURE) --prefix=$(shell pwd)/$(TARGET_DIRECTORY)

ifneq ($(call get_auto_configure_info,extra-cflags),)
CONFIGURE		:= $(CONFIGURE) --extra-cflags="$(COMPILE_FLAGS)"
else
ENVIRONMENT		:= $(ENVIRONMENT) CFLAGS="$(COMPILE_FLAGS)"
endif
ifneq ($(call get_auto_configure_info,extra-cxxflags),)
CONFIGURE		:= $(CONFIGURE) --extra-cxxflags="$(COMPILE_FLAGS)"
else
ENVIRONMENT		:= $(ENVIRONMENT) CXXFLAGS="$(COMPILE_FLAGS)"
endif
ifneq ($(call get_auto_configure_info,extra-ldflags),)
CONFIGURE		:= $(CONFIGURE) --extra-ldflags="$(LINK_FLAGS)"
else
ENVIRONMENT		:= $(ENVIRONMENT) LDFLAGS="$(COMPILE_FLAGS)"
endif

ifeq ($(CONFIG_LIB),static)
CONFIGURE		:= $(CONFIGURE) --enable-static --disable-shared
else
CONFIGURE		:= $(CONFIGURE) --disable-static --enable-shared
endif

# Cross-compilation:
ifneq ($(PLATFORM_TOOL_PREFIX),)
ifneq ($(call get_auto_configure_info,host),)
CONFIGURE		:= $(CONFIGURE) --host=$(BUILD_TARGET)
endif
ifneq ($(call get_auto_configure_info,cross-prefix),)
CONFIGURE		:= $(CONFIGURE) --cross-prefix=$(PLATFORM_TOOL_PREFIX)
else
ENVIRONMENT		:= $(ENVIRONMENT) CC=$(CC)
ENVIRONMENT		:= $(ENVIRONMENT) CXX=$(CXX)
endif
endif

ifneq ($(CONFIG_COMPILE),release)
ifneq ($(call get_auto_configure_info,enable-debug),)
	LOCAL_CONFIGURE		:= $(LOCAL_CONFIGURE) --enable-debug
endif
endif

CONFIGURE		:= $(CONFIGURE) $(LOCAL_CONFIGURE)

.PHONY: target
target: $(TARGET_FILE_FULL)

.PHONY: info
info:
	@$(ECHO) "Name: $(LOCAL_NAME)"
	@$(ECHO) "Type: $(LOCAL_TYPE)"
	@$(ECHO) "SourceDirectory: $(SOURCE_DIRECTORY)"
	@$(ECHO) "Buildable: $(BUILDABLE)"
	@$(ECHO) "TargetDirectory: $(dir $(TARGET_FILE_FULL))"
	@$(ECHO) "Target: $(notdir $(TARGET_FILE_FULL))"
	@$(ECHO) "Version: $(LOCAL_VERSION)"
	@$(ECHO) "File: $(TARGET_FILE_FULL)"
	@$(ECHO) "DependLibs: $(LOCAL_DEPEND_LIBRARYS)"

FILE_ACLOCAL		:= $(SOURCE_DIRECTORY)/aclocal.m4

FILE_CONFIGURE_AC	:= $(SOURCE_DIRECTORY)/configure.ac
FILE_CONFIGURE	:= $(SOURCE_DIRECTORY)/configure

FILE_MAKEFILE_AM	:= $(SOURCE_DIRECTORY)/Makefile.am
FILE_MAKEFILE_IN	:= $(SOURCE_DIRECTORY)/Makefile.in
FILE_MAKEFILE		:= $(SOURCE_DIRECTORY)/Makefile

ifneq ($(LOCAL_CONFIG_H),)
	FILE_CONFIG_H		:= $(SOURCE_DIRECTORY)/$(LOCAL_CONFIG_H)
endif

$(FILE_ACLOCAL):
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && libtoolize && aclocal

ifeq ($(wildcard $(FILE_CONFIGURE)),)
$(FILE_CONFIGURE): $(FILE_CONFIGURE_AC) $(FILE_ACLOCAL)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && autoconf
endif

ifeq ($(wildcard $(FILE_MAKEFILE_IN)),)
$(FILE_MAKEFILE_IN): $(FILE_MAKEFILE_AM) $(FILE_CONFIGURE_AC) $(FILE_ACLOCAL)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && automake -a
endif

ifneq ($(FILE_CONFIG_H),)
$(FILE_CONFIG_H): $(FILE_CONFIGURE)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(ENVIRONMENT) ./configure $(CONFIGURE)
endif

ifeq ($(wildcard $(FILE_MAKEFILE)),)
$(FILE_MAKEFILE): $(FILE_MAKEFILE_IN) $(FILE_CONFIGURE)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(ENVIRONMENT) ./configure $(CONFIGURE)
endif

ifeq ($(wildcard $(TARGET_DIRECTORY)),)
.PHONY: make_install
make_install: $(FILE_MAKEFILE) $(FILE_CONFIG_H)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(MAKE) install && $(MAKE) distclean

$(TARGET_FILE_FULL): make_install
endif
