PLATFORM_INCLUDE_DIRECTORY	:= $(PLATFORM_TOOL_DIRECTORY)/pal11lda3-src_20110404/usr/include

PLATFORM_LIB_DIRECTORY		:= $(PLATFORM_TOOL_DIRECTORY)/pal11lda3-src_20110404/usr/lib

PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORY) \
				$(PLATFORM_INCLUDE_DIRECTORY)/c++/4.2/

PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -mtarget=LDA3

PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -mtarget=LDA3
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -L$(PLATFORM_LIB_DIRECTORY)
PLATFORM_LINK_FLAGS            := $(PLATFORM_LINK_FLAGS) -Wl,-rpath,/usr/local/lib -Wl,-dy

PLATFORM_DISABLE_FLAGS		:= -Wl,--exclude-libs 
