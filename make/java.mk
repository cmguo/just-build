################################################################################
## @file:	java.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	管理工程的规则文件
## @version	1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

SOURCE_DIRECTORY	:= $(ROOT_DIRECTORY)$(LOCAL_NAME)

TARGET_FILE_FULL	:= $(LOCAL_TARGET)

SOURCE_SUB_DIRECTORYS	:= * */* */*/* */*/*/*

JAVA_FILES		:= $(foreach dir,$(SOURCE_SUB_DIRECTORYS),$(wildcard $(SOURCE_DIRECTORY)/$(dir)/*.java))
JAVA_FILES		:= $(patsubst $(SOURCE_DIRECTORY)/%,%,$(JAVA_FILES))
$(info JAVA_FILES=$(JAVA_FILES))
CLASS_FILES		:= $(patsubst %.java,%.class,$(JAVA_FILES))
$(info CLASS_FILES=$(CLASS_FILES))

TARGETS			:= target clean distclean info publish

.PHONY: target

target: $(TARGET_FILE_FULL)

$(CLASS_FILES): %.class: $(SOURCE_DIRECTORY)/%.java
	javac -d . $<

$(TARGET_FILE_FULL): $(CLASS_FILES)
	jar cf $@ $(CLASS_FILES)

