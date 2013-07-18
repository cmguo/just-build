################################################################################
## @file:	info.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	提取项目信息的函数
## @version	1.0
###############################################################################

# 通用提取项目信息
# argment1:	项目名称
# return:	该信息项的值

define get_auto_configure_info
$(strip \
	$(shell LANG=C cd $(SOURCE_DIRECTORY) && ./configure --help | \
		awk '{ if (match($$1, "--$(1)[=|$$]")) { $$1="" ; print $$0 } }') \
)
endef

