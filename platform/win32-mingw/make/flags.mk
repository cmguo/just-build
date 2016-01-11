PLATFORM_DISABLE_FLAGS		:= -fPIC -pthread -fvisibility

PLATFORM_COMPILE_FLAGS		:= -fno-strict-aliasing -Wno-unused

PLATFORM_LINK_FLAGS			:= -static-libgcc -static-libstdc++

DYNAMIC_NAME_SUFFIX		:= .dll

BIN_NAME_SUFFIX			:= .exe
