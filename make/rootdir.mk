################################################################################
## @file:	rootdir.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	根目录.
## @version	1.0
###############################################################################

RELETIVE_DIR		:= $(patsubst %/,%,$(dir $(RELETIVE_DIR)))
ROOT_DIRECTORY		:= $(patsubst %/,%,$(dir $(RELETIVE_DIR)))

ROOT_OUTPUT_DIRECTORY	:= $(ROOT_DIRECTORY)/output
ROOT_TEST_DIRECTORY		:= $(ROOT_DIRECTORY)/test
ROOT_STRATEGY_DIRECTORY 	:= $(ROOT_DIRECTORY)/strategy
ROOT_BUILD_DIRECTORY		:= $(ROOT_DIRECTORY)/build

ROOT_MAKE_DIRECTORY		:= $(ROOT_BUILD_DIRECTORY)/make
ROOT_PLATFORM_DIRECTORY	:= $(ROOT_BUILD_DIRECTORY)/platform
ROOT_TOOL_DIRECTORY		:= $(ROOT_BUILD_DIRECTORY)/tools

ROOT_LIBRARY_DIRECTORY	:= $(ROOT_OUTPUT_DIRECTORY)/lib
ROOT_BINARY_DIRECTORY	:= $(ROOT_OUTPUT_DIRECTORY)/bin
PROOT_ACKET_DIRECTORY	:= $(ROOT_OUTPUT_DIRECTORY)/pack
ROOT_PUBLISH_DIRECTORY	:= $(ROOT_OUTPUT_DIRECTORY)/pub
