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
	BUILD_HOST	:= $(strip $(shell PATH="$(PATH)" LANG=C $(CXX) -v 2>&1 | awk -F : '$$1 == "Target" { print $$2 }'))
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

ifneq ($(AUTO_SUB_TYPE),bin)

ifeq ($(CONFIG_LIB),static)
CONFIGURE_static	:= yes
CONFIGURE_shared	:= no
else
CONFIGURE_static	:= no
CONFIGURE_shared	:= yes
endif

ifneq ($(call get_auto_configure_info,enable-static),)
CONFIGURE		:= $(CONFIGURE) --enable-static=$(CONFIGURE_static)
endif
ifneq ($(call get_auto_configure_info,disable-shared),)
CONFIGURE		:= $(CONFIGURE) --disable-shared=$(CONFIGURE_static)
endif
ifneq ($(call get_auto_configure_info,disable-static),)
CONFIGURE		:= $(CONFIGURE) --disable-static=$(CONFIGURE_shared)
endif
ifneq ($(call get_auto_configure_info,enable-shared),)
CONFIGURE		:= $(CONFIGURE) --enable-shared=$(CONFIGURE_shared)
endif

endif # AUTO_SUB_TYPE != bin

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
ENVIRONMENT		:= $(ENVIRONMENT) $(AUTO_CONFIGURE_VARS)

ifneq ($(CONFIG_PROFILE),release)
ifneq ($(call get_auto_configure_info,enable-debug),)
AUTO_CONFIGURE		:= $(AUTO_CONFIGURE) --enable-debug
endif
endif

CONFIGURE		:= $(CONFIGURE) $(AUTO_CONFIGURE)

FILE_CONFIGURE_AC	:= $(SOURCE_DIRECTORY)/configure.ac
FILE_CONFIGURE_IN	:= $(SOURCE_DIRECTORY)/configure.in
FILE_CONFIGURE		:= $(SOURCE_DIRECTORY)/configure

FILE_MAKEFILE_AM	:= $(SOURCE_DIRECTORY)/Makefile.am
FILE_MAKEFILE_IN	:= $(SOURCE_DIRECTORY)/Makefile.in
FILE_MAKEFILE		:= $(SOURCE_DIRECTORY)/Makefile

ifneq ($(AUTO_CONFIG_H),)
FILE_CONFIG_H		:= $(SOURCE_DIRECTORY)/$(AUTO_CONFIG_H)
endif

ifeq ($(wildcard $(FILE_CONFIGURE)),)
$(FILE_CONFIGURE): 
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && autoreconf -fvi && rm -rf autom4te*.cache
endif

ifeq ($(wildcard $(FILE_MAKEFILE)),)
$(FILE_MAKEFILE): $(FILE_CONFIGURE)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(ENVIRONMENT) ./configure $(CONFIGURE)
else

ifneq ($(FILE_CONFIG_H),)
$(FILE_CONFIG_H): $(FILE_CONFIGURE)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(ENVIRONMENT) ./configure $(CONFIGURE)
endif

endif

AUTO_TARGET_		:= $(addprefix lib/$(NAME_PREFIX),$(PROJECT_TARGET))

ifeq ($(NAME_SUFFIX),.dll)
	TARGET_FILE_FULL2		:= $(TARGET_FILE:$(DYNAMIC_NAME_PREFIX)%=$(TARGET_DIRECTORY)/$(STATIC_NAME_PREFIX)%.a)
	AUTO_TARGET_		:= $(addprefix lib/$(STATIC_NAME_PREFIX),$(PROJECT_TARGET))
	AUTO_TARGET_		:= $(AUTO_TARGET_) $(addprefix bin/*,$(PROJECT_TARGET))
endif

AUTO_TARGET_		:= $(addsuffix *,$(AUTO_TARGET_))
AUTO_TARGET_ALL	:= $(wildcard $(addprefix $(TARGET_DIRECTORY)/,$(AUTO_TARGET_)))

AUTO_TARGET_UP_ALL	:= $(wildcard $(TARGET_DIRECTORY)/*.*)
AUTO_TARGET_UP_ALL	:= $(filter-out $(TARGET_FILE_FULL) $(TARGET_FILE_FULL2),$(AUTO_TARGET_UP_ALL))

.PHONY: make_install
make_install: $(FILE_MAKEFILE) $(FILE_CONFIG_H)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(MAKE) install
	$(CD) $(SOURCE_DIRECTORY) && $(MAKE) distclean
	$(call repo_clean,$(SOURCE_DIRECTORY))

ifeq ($(AUTO_TARGET_ALL),)
AUTO_TARGET_ALL 	:= make_install
endif

.PHONY: link_up
link_up: $(AUTO_TARGET_ALL)
	$(CD) $(TARGET_DIRECTORY) && $(LN) -s $(AUTO_TARGET_) .

ifeq ($(AUTO_TARGET_UP_ALL),)
AUTO_TARGET_UP_ALL 	:= link_up
endif

target: $(TARGET_FILE_FULL2)

$(TARGET_FILE_FULL): $(AUTO_TARGET_UP_ALL)
	$(RM) $@ && $(CD) $(TARGET_DIRECTORY) && $(LN) -s $(@:$(TARGET_DIRECTORY)/$(NAME_PREFIX)%$(NAME_SUFFIX_FULL)=*%*$(NAME_SUFFIX)) $(@:$(TARGET_DIRECTORY)/%=%)

$(TARGET_FILE_FULL2): $(AUTO_TARGET_UP_ALL)
	$(RM) $@ && $(LN) $(@:%$(NAME_SUFFIX_FULL).a=%*$(NAME_SUFFIX).a) $@
