################################################################################
## @file:	base.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	基本函数扩展
## @version	1.0
###############################################################################

# 反转列表
# argment1:	原列表
# return:	反转后的列表

revert			= $(if $1,$(call revert,$(wordlist 2,$(words $1),$1)) $(firstword $1))

# 连接列表为整串
# argment1:	连接符
# argment2:	列表
# return:	合并后的整串

joinlist		= $(if $(word 2,$2),$(join $(firstword $2)$1,$(call joinlist,$1,$(wordlist 2,$(words $2),$2))),$2)

# 切割整串为列表
# argment1:	分隔符
# argment2:	整串
# return:	切割后的列表

split			= $(subst $1, ,$2)


# 判断两个值是否完全相等
# argment1:	值1
# argment2:	值2
# return:	如果相等：OK，否则：空

equal			= $(shell if [ \"$1\" = \"$2\" ] ; then echo \"OK\" ; fi)

# 输出变量信息
# argment1: 变量名数组
# return:	无

dump			= $(foreach v,$1,$(info $2$(v)=$($(v))))

# 去除列表中重复的元素
# # argment1: 列表
# # return:   去重后的列表

uniq			= $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
