PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/sources/cxx-stl/gnu-libstdc++/include
PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/sources/cxx-stl/gnu-libstdc++/libs/x86/include
PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/platforms/android-9/arch-x86/usr/include

#PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/external/stlport/stlport
#PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/bionic
#PLATFORM_INCLUDE_DIRECTORYS	:= $(PLATFORM_INCLUDE_DIRECTORYS) /android-froyo/ndk/build/platforms/android-8/arch-x86/usr/include

PLATFORM_LIBRARY_DIRECTORYS	:= $(PLATFORM_LIBRARY_DIRECTORYS) /android-froyo/sources/cxx-stl/gnu-libstdc++/libs/x86
PLATFORM_LIBRARY_DIRECTORYS	:= $(PLATFORM_LIBRARY_DIRECTORYS) /android-froyo/platforms/android-9/arch-x86/usr/lib

#PLATFORM_LIBRARY_DIRECTORYS	:= $(PLATFORM_LIBRARY_DIRECTORYS) /android-froyo/ndk/build/platforms/android-8/arch-x86/usr/lib

PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fpic
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -ffunction-sections
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -funwind-tables
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fstack-protector
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fno-short-enums 
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -mhard-float
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fomit-frame-pointer
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -fno-strict-aliasing
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -finline-limit=64
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -Wa,--noexecstack
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -DANDROID 

PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -D__ANDROID__=1

PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -nostdlib
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Bdynamic
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-dynamic-linker,/system/bin/linker
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,--gc-sections
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-z,nocopyreloc
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,--no-undefined
PLATFORM_LINK_FLAGS		:= $(PLATFORM_LINK_FLAGS) -Wl,-z,noexecstack

PLATFORM_CRTBEGIN_STATICBIN	:= crtbegin_static.o
PLATFORM_CRTEND_STATICBIN	:= crtend_android.o
PLATFORM_CRTBEGIN_DYNAMICBIN	:= crtbegin_dynamic.o
PLATFORM_CRTEND_DYNAMICBIN	:= crtend_android.o
PLATFORM_CRTBEGIN_DYNAMIC	:= crtbegin_so.o
PLATFORM_CRTEND_DYNAMIC		:= crtend_so.o

PLATFORM_DEPEND_LIBRARYS	:= $(PLATFORM_DEPEND_LIBRARYS) gnustl_shared 
PLATFORM_DEPEND_LIBRARYS	:= $(PLATFORM_DEPEND_LIBRARYS) m rt pthread c dl gcc

PLATFORM_DISABLE_FLAGS		:= -pthread
