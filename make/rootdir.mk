################################################################################
## @file:	rootdir.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	根目录.
## @version	1.0
###############################################################################

ROOT_DIRECTORY		:= $(dir $(RELETIVE_DIR))
ROOT_SOURCE_DIRECTORY	:= $(ROOT_DIRECTORY)src
ROOT_HEADER_DIRECTORY	:= $(ROOT_DIRECTORY)include
ROOT_LIBRARY_DIRECTORY	:= $(ROOT_DIRECTORY)lib
ROOT_BINARY_DIRECTORY	:= $(ROOT_DIRECTORY)bin
ROOT_PACKET_DIRECTORY	:= $(ROOT_DIRECTORY)pack
ROOT_PUBLISH_DIRECTORY	:= $(ROOT_DIRECTORY)pub
ROOT_PROJECT_DIRECTORY	:= $(ROOT_DIRECTORY)project
ROOT_STRATEGY_DIRECTORY := $(ROOT_DIRECTORY)strategy
ROOT_BUILD_DIRECTORY	:= $(ROOT_DIRECTORY)build
ROOT_MAKE_DIRECTORY	:= $(ROOT_DIRECTORY)make
ROOT_PLATFORM_DIRECTORY	:= $(ROOT_DIRECTORY)platform
ROOT_TOOL_DIRECTORY	:= $(ROOT_DIRECTORY)tools
ROOT_TEST_DIRECTORY	:= $(ROOT_DIRECTORY)test
