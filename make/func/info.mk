################################################################################
## @file:	info.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	提取项目信息的函数
## @version	1.0
###############################################################################

# 通用提取项目信息
# argment1:	项目名称
# argment2: 需要提取的信息项
# return:	该信息项的值

define get_item_info
$(strip \
	$(shell $(MAKE) LOCAL_NAME=$1 config="$(strip $(config))" info | \
		awk -F : '{ if ($$1 == "$2") print $$2 }') \
)
endef

# 通用提取项目信息（多项，可以指定分隔符）
# argment1:	项目名称
# argment2: 多项信息的分隔符
# argment3: 需求提取的信息项，多项
# return:	提取的信息项

define get_item_infos
$(strip \
	$(shell $(MAKE) LOCAL_NAME=$1 config="$(strip $(config))" info | \
                awk -F : ' 
                        BEGIN { $(foreach info,$3,infos["$(info)"];) } 
                        { if ($$1 in infos) infos["$$1"] = $$2; }
                        END { $(foreach info,$3,print infos["$(info)"];print $2) }') \
)
endef

# 通用提取项目信息（格式化）
# argment1:	项目名称
# argment2: 格式化字符串（{A},{B}:{C}）
# return:	格式化结果

define get_item_info_format
$(strip \
	$(shell $(MAKE) LOCAL_NAME=$1 config="$(strip $(config))" info | \
                awk -F : ' 
                        BEGIN { result="$2" } 
                        { 
							regex="{"$$1"}";
							sub(/^[[:blank:]]*/,"",$$2);
							sub(/[[:blank:]]*$$/,"",$$2);
							gsub(regex,$$2,result);
						}
                        END { print result; }') \
)
endef
# 通用提取项目信息到变量中（可以同时多项）
# argment1:	项目名称
# argment2: 提取的信息项（多项）
# argment3: 变量名后缀（默认为项目名，/转换为_）
# return:	变量名称列表

define get_item_info_as_var
$(eval get_item_info_as_var_ret:=$(strip \
	$(shell $(MAKE) LOCAL_NAME=$1 config="$(strip $(config))" info | \
		awk -F : ' 
			BEGIN { $(foreach info,$2,infos["$(info)"];) suffix="$(if $3,$3,$(subst /,_,$1))"; } 
			{ if ($$1 in infos) printf "%s%s $$(eval %s%s:=%s)", $$1, suffix, $$1, suffix, $$2; } 
		' \
	) \
))$(get_item_info_as_var_ret)
endef


# 提取项目类型
# argment1:	项目名称
# return:	项目类型

define get_item_type
$(call get_item_info,$1,Type)
endef

# 提取项目目标文件
# argment1:	项目名称
# return:	项目类型

define get_item_file
$(call get_item_info,$1,File)
endef

# 提取工程直接依赖项
# 提取工程直接依赖项
# argment1:	项目名称
# return:	项目直接依赖项

define get_item_depends
$(call get_item_info,$1,Depends)
endef
