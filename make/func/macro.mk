################################################################################
## @file:	info.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	提取项目信息的函数
## @version	1.0
###############################################################################

# 通用提取宏定义
# argment1:     文件名
# argment2:     提取的信息项
# return:       该信息项的值

define get_macro_info
$(strip \
        $(shell PATH=$(PATH) LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(CC) -E $(1) $(addprefix -I,$(wildcard $(HEADER_DIRECTORYS))) -dM | \
		awk '$$2 == "$(2)" { gsub("\"","",$$3); print $$3; }') \
)
endef

