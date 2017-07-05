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
BUILD_VERSION		:= $(BUILD_VERSION:$(VERSION)_%=%)

ifneq ($(CONFIG_packet),)
	NAME_PACKET_STRATEGY	:= ($(CONFIG_packet))
endif

TARGET_FILE             := $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONFIG_PROFILE)$(NAME_PACKET_STRATEGY)-$(VERSION).tar.gz
TARGET_FILE_SYMBOL		:= $(patsubst %.tar.gz,%-symblo.tar.gz,$(TARGET_FILE))

TARGET_FILE_FULL        := $(TARGET_DIRECTORY)/$(TARGET_FILE)
TARGET_FILE_SYMBOL_FULL	:= $(TARGET_DIRECTORY)/$(TARGET_FILE_SYMBOL)
