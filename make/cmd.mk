################################################################################
## @file:	cmd.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定各种命令.
## @version	1.0
###############################################################################

ifeq ($(AR),)
	AR		:= ar
endif
AR		:= $(PLATFORM_TOOL_PREFIX)$(AR)

ifeq ($(AS),)
	AS		:= as
endif
AS		:= $(PLATFORM_TOOL_PREFIX)$(AS)

ifeq ($(CC),)
	CC		:= gcc
endif
CC		:= $(PLATFORM_TOOL_PREFIX)$(CC)

ifeq ($(CXX),)
	CXX		:= g++
endif
CXX		:= $(PLATFORM_TOOL_PREFIX)$(CXX)

ifeq ($(LD),ld)
	LD		:= $(CXX)
endif
ifeq ($(LD),)
	LD		:= $(CXX)
endif
LD		:= $(PLATFORM_TOOL_PREFIX)$(LD)

ifeq ($(LIBTOOL),)
	LIBTOOL		:= libtool
endif
LIBTOOL		:= $(PLATFORM_TOOL_PREFIX)$(LIBTOOL)

ifeq ($(STRIP),)
	STRIP		:= strip -s
endif
STRIP		:= $(PLATFORM_TOOL_PREFIX)$(STRIP)

ifeq ($(CD),)
	CD	        := cd
endif

ifeq ($(RM),)
	RM		:= rm -fr
endif

ifeq ($(RM),rm -f)
	RM		:= rm -fr
endif

ifeq ($(LN),)
	LN		:= ln -f
endif

ifeq ($(CP),)
	CP		:= cp -f
endif

ifeq ($(MV),)
	MV		:= mv
endif

ifeq ($(MKDIR),)
	MKDIR		:= mkdir -p
endif

ifeq ($(ECHO),)
	ECHO		:= echo
endif

ifeq ($(SHELL),)	
	SHELL		:= /bin/sh
endif

ifeq ($(CTARGS),)
	CTARGS		:= ctags --extra=+fq \
	--c++-types=cfgmnpstu --append=yes 
endif

ifeq ($(CSCOPE_CMD),)
	CSCOPE_CMD	:= cscope -b
endif

ifeq ($(MAKE),)
	MAKE		:= make
endif

ifeq ($(SED),)
	SED		:= sed
endif

ifeq ($(EV),)
	EV		:= file_version
endif
