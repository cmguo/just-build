################################################################################
## @file:	cmd.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	设定各种命令.
## @version	1.0
###############################################################################
ifeq ($(AR),ar)
	AR		:= $(PLATFORM_TOOL_PREFIX)ar
endif

ifeq ($(AS),as)
	AS		:= $(PLATFORM_TOOL_PREFIX)as
endif

ifeq ($(CC),cc)
	CC		:= $(PLATFORM_TOOL_PREFIX)g++
endif

ifeq ($(CXX),g++)
	CXX		:= $(PLATFORM_TOOL_PREFIX)g++
endif

ifeq ($(LD),ld)
	LD		:= $(CC)
endif

ifeq ($(STRIP),)
	STRIP		:= $(PLATFORM_TOOL_PREFIX)strip -s
endif

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
