################################################################################
## @file:	call.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	一些特殊的函数调用方式
## @version	1.0
###############################################################################

# 帮助以数组的形式传递参数
# argment1:	实际要调用的函数名
# argment2:	参数数组，以逗号分隔
# return:       实际函数调用结果

call_argv       = $(eval call_argv_result := $$(call $1,$2))$(call_argv_result)
