################################################################################
## @file:   config.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	发布包的配置
## @version	1.0
###############################################################################

CONFIG				:= $(config) $(PACKET_CONFIG)

CONFIG_PROFILE		:= $(call get_config,$(CONFIG),debug release,debug)

TARGET_DIRECTORY	:= $(CONFIG_PROFILE)

VERSION				:= $(call get_item_info,$(PACKET_VERSION_DEPEND),Version)
VERSION_DEPEND		:= $(call get_item_info,$(PACKET_VERSION_DEPEND),File)
VERSION_DEPEND		:= $(PLATFORM_OUTPUT_DIRECTORY)$(PACKET_VERSION_DEPEND)/$(VERSION_DEPEND) 

VERSION_ALL			:= $(strip $(shell $(EV) $(VERSION_DEPEND) $(call get_item_info,$(PACKET_VERSION_DEPEND),Target)))
BUILD_VERSION		:= $(VERSION_ALL:$(VERSION)_%=%)

ifneq ($(CONFIG_packet),)
	NAME_PACKET_STRATEGY	:= ($(CONFIG_packet))
endif

ifeq ($(PACKET_TYPE),tar.gz)
	PACK	:= tar -czv -f 
else
ifeq ($(PACKET_TYPE),zip)
	PACK	:= zip -j -r 
else
	PACKET_TYPE	:= tar.gz
	PACK    := tar -czv -f
endif
endif

TARGET_FILE             := $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONFIG_PROFILE)$(NAME_PACKET_STRATEGY)-$(VERSION_ALL).$(PACKET_TYPE)
TARGET_FILE_SYMBOL		:= $(patsubst %.$(PACKET_TYPE),%-symblo.$(PACKET_TYPE),$(TARGET_FILE))

TARGET_FILE_FULL        := $(TARGET_DIRECTORY)/$(TARGET_FILE)
TARGET_FILE_SYMBOL_FULL	:= $(TARGET_DIRECTORY)/$(TARGET_FILE_SYMBOL)
