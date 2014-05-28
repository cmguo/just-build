################################################################################
## @file:	depends.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief   沿着项目依赖关系遍历项目
## @version     1.0
###############################################################################

include $(ROOT_MAKE_DIRECTORY)/func/info.mk

depend_cut			= $(shell echo '$1' | cut -d '|' -f $2)

# 递归遍历辅助回调函数
# argment1:	访问回调函数
# argment2:	当前项目
# argment3:	提取的信息项

depend_callback		= $(eval depend_callback_ret:=$$(call $1,$2,$3))$(depend_callback_ret)

# 递归遍历辅助递归函数4
# argment1: 访问回调函数
# argment2: 需要提取信息项（包括当前项，格式化字符串，如：{Depend}|{Type},{File}）
# argment3: 已发现还没有访问的项目
# argment4: 已经遍历的项目
# return:	回调函数返回值合并

depend_visit_4		= $(if $3,$(call depend_visit_2,$1,$2,$(firstword $3),$3,$4))

# 递归遍历辅助递归函数3
# argment1: 访问回调函数
# argment2: 需要提取信息项（包括当前项，格式化字符串，如：{Depend}|{Type},{File}）
# argment3: 当前项目
# argment4: 提取的信息项（包括依赖项目）
# argment5: 已发现还没有访问的项目（包括当前项）
# argment6: 已经遍历的项目（不包括当前项）
# return:	回调函数返回值合并

depend_visit_3		= $(call depend_callback,$1,$3,$(call depend_cut,$4,2))$(call depend_visit_4,$1,$2,$(filter-out $6 $3,$5 $(call depend_cut,$4,1)),$6 $3)

# 递归遍历辅助递归函数2
# argment1: 访问回调函数
# argment2: 需要提取信息项（包括当前项，格式化字符串，如：{Depend}|{Type},{File}）
# argment3: 当前项目
# argment4: 已发现还没有访问的项目（包括当前项）
# argment5: 已经遍历的项目（不包括当前项）
# return:	回调函数返回值合并

depend_visit_2		= $(call depend_visit_3,$1,$2,$3,$(call get_item_info_format,$3,$2),$4,$5)

# 递归遍历
# argment1: 根项目（可以多个，在递归调用时表示已发现还没有访问的项目）
# argment2: 访问回调函数
# argment3: 需要提取信息项
# return:	回调函数返回值合并

COMMA	:=,

depend_visit		= $(strip $(call depend_visit_2,$2,{Depends}|$(call joinlist,$(COMMA),$(patsubst %,{%},$3)),$(firstword $1),$1,))
