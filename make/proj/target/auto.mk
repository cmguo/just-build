################################################################################
## @file:	auto.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	管理工程的规则文件
## @version	1.0
###############################################################################

COMPILE_FLAGS		:= $(filter-out -fvisibility%,$(COMPILE_FLAGS))

# we need update sysroot to absolute path
INC_PATHS		:= $(call reletive_directory,$(INC_PATHS))
LIB_PATHS		:= $(call reletive_directory,$(LIB_PATHS))

COMPILE_FLAGS		:= $(COMPILE_FLAGS) $(addprefix -I,$(INC_PATHS))

LINK_FLAGS		:= $(LINK_FLAGS) $(addprefix -L,$(LIB_PATHS))
LINK_FLAGS		:= $(LINK_FLAGS) $(LINK_LIB_NAMES)

BUILD_HOST		:= $(PLATFORM_TOOL_TARGET)
ifeq ($(BUILD_HOST),)
	BUILD_HOST	:= $(strip $(shell PATH=$(PATH) LANG=C $(PLATFORM_TOOL_PREFIX)gcc -v 2>&1 | awk -F : '$$1 == "Target" { print $$2 }'))
endif

# 通用提取项目信息
# argment1:	项目名称
# return:	该信息项的值

define get_auto_configure_info
$(strip \
	$(shell LANG=C cd $(SOURCE_DIRECTORY) && ./configure --help | \
		awk '{ if (match($$1, "--$(1)([=\\[]|$$)")) { $$1="" ; print $$0 } }') \
)
endef

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
ENVIRONMENT		:= $(ENVIRONMENT) LDFLAGS="$(LINK_FLAGS)"
endif

ifneq ($(LOCAL_SUB_TYPE),bin)
ifeq ($(CONFIG_LIB),static)
CONFIGURE		:= $(CONFIGURE) --enable-static --disable-shared
else
CONFIGURE		:= $(CONFIGURE) --disable-static --enable-shared
endif
endif

# Cross-compilation:
ifneq ($(PLATFORM_TOOL_PREFIX),)
ifneq ($(call get_auto_configure_info,build),)
#CONFIGURE		:= $(CONFIGURE) --build=$(BUILD_HOST)
endif
ifneq ($(call get_auto_configure_info,host),)
CONFIGURE		:= $(CONFIGURE) --host=$(BUILD_HOST)
endif
ifneq ($(call get_auto_configure_info,cross-prefix),)
CONFIGURE		:= $(CONFIGURE) --cross-prefix=$(PLATFORM_TOOL_PREFIX)
else
ENVIRONMENT		:= $(ENVIRONMENT) CC=$(CC)
ENVIRONMENT		:= $(ENVIRONMENT) CXX=$(CXX)
endif
endif
ENVIRONMENT		:= $(ENVIRONMENT) $(LOCAL_CONFIGURE_VARS)

ifneq ($(CONFIG_COMPILE),release)
ifneq ($(call get_auto_configure_info,enable-debug),)
LOCAL_CONFIGURE		:= $(LOCAL_CONFIGURE) --enable-debug
endif
endif

CONFIGURE		:= $(CONFIGURE) $(LOCAL_CONFIGURE)

FILE_ACLOCAL		:= $(SOURCE_DIRECTORY)/aclocal.m4

FILE_CONFIGURE_AC	:= $(SOURCE_DIRECTORY)/configure.ac
FILE_CONFIGURE		:= $(SOURCE_DIRECTORY)/configure

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
ifneq ($(wildcard $(FILE_CONFIGURE_AC)),)
$(FILE_CONFIGURE): $(FILE_CONFIGURE_AC) $(FILE_ACLOCAL)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && autoconf && rm -rf autom4te*.cache
else
$(FILE_CONFIGURE): 
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && autoreconf -fvi && rm -rf autom4te*.cache
endif
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

ifeq ($(LOCAL_TARGET),)
LOCAL_TARGET		:= $(addprefix lib/$(NAME_PREFIX),$(addsuffix $(NAME_SUFFIX),$(PROJECT_TARGET)))
endif

LOCAL_TARGET1		:= $(addprefix $(TARGET_DIRECTORY)/,$(LOCAL_TARGET))
LOCAL_TARGET2		:= $(addprefix $(TARGET_DIRECTORY)/,$(notdir $(LOCAL_TARGET)))

.PHONY: make_install
make_install: $(FILE_MAKEFILE) $(FILE_CONFIG_H)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(MAKE) install && $(MAKE) distclean
	$(CD) $(TARGET_DIRECTORY) && $(LN) -s $(addsuffix *,$(LOCAL_TARGET)) .

ifeq ($(wildcard $(LOCAL_TARGET1)),)

$(LOCAL_TARGET1): make_install

endif

ifeq ($(wildcard $(TARGET_FILE_FULL)),)

$(TARGET_FILE_FULL): $(LOCAL_TARGET1)
	$(LN) $(filter $(patsubst %$(NAME_SUFFIX_FULL),%,$@)%,$(LOCAL_TARGET2)) $@

endif
