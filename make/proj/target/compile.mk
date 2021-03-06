################################################################################
## @file:	compile.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	编译规则
## @version	1.0
###############################################################################

INCLUDES		:= $(INC_PATHS)

#$(warning $(INCLUDES))

ifneq ($(HEADERS),)
	INCLUDES		:= $(OBJECT_DIRECTORY) $(INCLUDES)
endif

INCLUDES		:= $(addprefix -I,$(INCLUDES))
PKG_CONFIG_INC_PATHS := $(addprefix -I,$(PKG_CONFIG_INC_PATHS))
INCLUDES := $(INCLUDES) $(PKG_CONFIG_INC_PATHS)

vpath %.h $(patsubst -I%,%,$(INCLUDES))

SOURCE_DEPENDS		:= $(addprefix $(DEPEND_DIRECTORY)/,$(SOURCE_DEPENDS))

HEADER_DEPENDS		:= $(addprefix $(DEPEND_DIRECTORY)/,$(HEADER_DEPENDS))

$(SOURCE_OBJECTS): %.o: $(SOURCE_DIRECTORY)/%$(SOURCE_SUFFIX) $(HEADER_OBJECTS) $(MAKEFILE_LIST) | prev_build
	@echo CXX: $@
	@$(CXX) $(COMPILE_FLAGS) $(INCLUDES) -c $< -o $(OBJECT_DIRECTORY)/$@ 
	@$(CXX) $(COMPILE_FLAGS) $(INCLUDES) -MM $< -MP -MT $@ -MF $(@:%.o=$(DEPEND_DIRECTORY)/%.dep)

$(HEADER_OBJECTS): %.gch : % $(MAKEFILE_LIST) | prev_build
	@echo CXX: $@
	@$(CXX) $(COMPILE_FLAGS) $(INCLUDES) -c $< -o $(OBJECT_DIRECTORY)/$@
	@$(CXX) $(COMPILE_FLAGS) $(INCLUDES) -MM $< -MP -MT $@ -MF $(@:%.gch=$(DEPEND_DIRECTORY)/%.dep)

-include $(HEADER_DEPENDS) $(SOURCE_DEPENDS)
