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

define get_conf
$(strip \
	$(shell LANG=C cd $(SOURCE_DIRECTORY) && ./configure --help | \
		awk '{ if (match($$1, "--$(1)(\\[=[A-Z]+\\]|=[A-Z]+|)$$")) { print $$1 } }') \
)
endef

define set_conf
$(strip \
	$(shell LANG=C cd $(SOURCE_DIRECTORY) && ./configure --help | \
	awk '{ if (match($$1, "--$(1)(\\[=[A-Z]+\\]|=[A-Z]+|)$$", result)) { if (!result[1]) print "--$(1)"; } }') \
)
endef

define set_conf_value
$(strip \
	$(shell LANG=C cd $(SOURCE_DIRECTORY) && ./configure --help | \
	awk '{ if (match($$1, "--$(1)(\\[=[A-Z]+\\]|=[A-Z]+|)$$", result)) { if (result[1]) print "--$(1)=$(2)"; } }') \
)
endef

ENVIRONMENT		:= 

CONFIGURE		:=

# Standard options:
CONFIGURE		:= $(CONFIGURE) --prefix=$(shell pwd)/$(TARGET_DIRECTORY)

ifneq ($(call get_conf,extra-cflags),)
CONFIGURE		:= $(CONFIGURE) --extra-cflags="$(COMPILE_FLAGS)"
else
CFLAGS			:= $(COMPILE_FLAGS)
ENVIRONMENT		+= CFLAGS
endif
ifneq ($(call get_conf,extra-cxxflags),)
CONFIGURE		:= $(CONFIGURE) --extra-cxxflags="$(COMPILE_FLAGS)"
else
CXXFLAGS		:= $(COMPILE_FLAGS)
ENVIRONMENT		+= CXXFLAGS
endif
ifneq ($(call get_conf,extra-ldflags),)
CONFIGURE		:= $(CONFIGURE) --extra-ldflags="$(LINK_FLAGS)"
else
LDFLAGS			:= $(LINK_FLAGS)
ENVIRONMENT		+= LDFLAGS
endif

ifneq ($(AUTO_SUB_TYPE),bin)

ifeq ($(CONFIG_LIB),static)
CONFIGURE_static	:= yes
CONFIGURE_shared	:= no
CONFIGURE		:= $(CONFIGURE) $(call set_conf,enable-static)
CONFIGURE		:= $(CONFIGURE) $(call set_conf,disable-shared)
else
CONFIGURE_static	:= no
CONFIGURE_shared	:= yes
CONFIGURE		:= $(CONFIGURE) $(call set_conf,disable-static)
CONFIGURE		:= $(CONFIGURE) $(call set_conf,enable-shared)
endif

CONFIGURE		:= $(CONFIGURE) $(call set_conf_value,enable-static,$(CONFIGURE_static))
CONFIGURE		:= $(CONFIGURE) $(call set_conf_value,enable-shared,$(CONFIGURE_shared))

endif # AUTO_SUB_TYPE != bin

# Cross-compilation:
ifneq ($(PLATFORM_TOOL_PREFIX),)
ifneq ($(call get_conf,build),)
#CONFIGURE		:= $(CONFIGURE) --build=$(BUILD_HOST)
endif
ifneq ($(call get_conf,host),)
CONFIGURE		:= $(CONFIGURE) --host=$(BUILD_HOST)
endif
ifneq ($(call get_conf,cross-prefix),)
CONFIGURE		:= $(CONFIGURE) --cross-prefix=$(PLATFORM_TOOL_PREFIX)
else
ENVIRONMENT		+= CC
ENVIRONMENT		+= CXX
endif
endif # $(PLATFORM_TOOL_PREFIX)

ENVIRONMENT		+= $(AUTO_CONFIGURE_VARS)

ifneq ($(CONFIG_PROFILE),release)
CONFIGURE		:= $(CONFIGURE) $(call set_conf,enable-debug)
endif

CONFIGURE		:= $(CONFIGURE) $(AUTO_CONFIGURE)

FILE_CONFIGURE_AC	:= $(SOURCE_DIRECTORY)/configure.ac
FILE_CONFIGURE_IN	:= $(SOURCE_DIRECTORY)/configure.in
FILE_CONFIGURE		:= $(SOURCE_DIRECTORY)/configure

FILE_MAKEFILE_AM	:= $(SOURCE_DIRECTORY)/Makefile.am
FILE_MAKEFILE_IN	:= $(SOURCE_DIRECTORY)/Makefile.in
FILE_MAKEFILE		:= $(SOURCE_DIRECTORY)/Makefile

# for pkg-config
PKG_CONFIG_LIBDIR	:=
PKG_CONFIG_PATH		:= $(call joinlist,:,$(LIB_PATHS:%=%/lib/pkgconfig))
ENVIRONMENT			+= PKG_CONFIG_LIBDIR PKG_CONFIG_PATH

ifneq ($(AUTO_CONFIG_H),)
FILE_CONFIG_H		:= $(SOURCE_DIRECTORY)/$(AUTO_CONFIG_H)
endif

ENVIRONMENT		:= $(foreach e,$(ENVIRONMENT),$(e)="$($(e))")

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
AUTO_TARGET			:= $(AUTO_TARGET_:%=$(TARGET_DIRECTORY)/%$(NAME_SUFFIX))

ifeq ($(NAME_SUFFIX),.dll)
	TARGET_FILE_FULL2		:= $(TARGET_NAME_FULL:$(DYNAMIC_NAME_PREFIX)%=$(TARGET_DIRECTORY)/$(STATIC_NAME_PREFIX)%.a)
	AUTO_TARGET_		:= $(addprefix lib/$(STATIC_NAME_PREFIX),$(PROJECT_TARGET))
	AUTO_TARGET_		:= $(AUTO_TARGET_) $(addprefix bin/*,$(PROJECT_TARGET))
endif

AUTO_TARGET_		:= $(addsuffix *,$(AUTO_TARGET_))
AUTO_TARGET_ALL		:= $(wildcard $(addprefix $(TARGET_DIRECTORY)/,$(AUTO_TARGET_)))

AUTO_TARGET_UP		:= $(PROJECT_TARGET:%=$(TARGET_DIRECTORY)/$(NAME_PREFIX)%$(NAME_SUFFIX))
AUTO_TARGET_UP_ALL	:= $(wildcard $(TARGET_DIRECTORY)/*.*)
AUTO_TARGET_UP_ALL	:= $(filter-out $(TARGET_FILE_FULL) $(TARGET_FILE_FULL2),$(AUTO_TARGET_UP_ALL))

.PHONY: make_install
make_install: $(FILE_MAKEFILE) $(FILE_CONFIG_H)
	@echo $@
	$(CD) $(SOURCE_DIRECTORY) && $(MAKE) install
	#$(CD) $(SOURCE_DIRECTORY) && $(MAKE) distclean
	$(call repo_clean,$(SOURCE_DIRECTORY))

ifeq ($(AUTO_TARGET_ALL),)
AUTO_TARGET_ALL 	:= make_install
endif

ifneq ($(AUTO_TARGET),$(wildcard $(AUTO_TARGET)))
$(AUTO_TARGET): make_install
endif

.PHONY: link_up
link_up: $(AUTO_TARGET)
	$(CD) $(TARGET_DIRECTORY) && $(LN) -sf $(AUTO_TARGET_) .

ifeq ($(AUTO_TARGET_UP_ALL),)
AUTO_TARGET_UP_ALL 	:= link_up
endif

ifneq ($(AUTO_TARGET_UP),$(wildcard $(AUTO_TARGET_UP)))
$(AUTO_TARGET_UP): link_up
endif

target: $(TARGET_FILE_FULL2)

$(TARGET_FILE_FULL): %$(NAME_SUFFIX_FULL) : %$(NAME_SUFFIX)
	$(RM) $@ && $(CD) $(TARGET_DIRECTORY) \
		&& $(LN) -s $(<:$(TARGET_DIRECTORY)/%=%) $(@:$(TARGET_DIRECTORY)/%=%)

$(TARGET_FILE_FULL2): $(AUTO_TARGET_UP_ALL)
	$(RM) $@ && $(LN) $(@:%$(NAME_SUFFIX_FULL).a=%*$(NAME_SUFFIX).a) $@
