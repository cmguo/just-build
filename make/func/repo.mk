################################################################################
## @file:	repo.mk
## @author	郭春茂 <gcm.ustc.edu>
## @brief	处理代码仓库的函数
## @version	1.0
###############################################################################

REPO_TYPE		:= none
ifneq ($(wildcard $(ROOT_BUILD_DIRECTORY)/.git),)
	REPO_TYPE		:= git
endif
ifneq ($(wildcard $(ROOT_BUILD_DIRECTORY)/.svn),)
	REPO_TYPE		:= svn
endif


# 获取版本仓库的版本号
# argment1:	仓库位置
# return:	版本号

repo_version_git	= $(shell git --git-dir $1/.git rev-parse --short HEAD)

repo_version_svn	= $(shell LANG=C svn info $1 | awk -F : '$$1 == "Revision" { print $$2}' 2> /dev/null)

repo_version		= $(call repo_version_$(REPO_TYPE), $1)

# 清理编译：删除临时文件，恢复被修改文件
# argment1:	仓库位置
# return:	无

repo_clean_git		= $(CD) $1 && git clean -f -d && git checkout .

repo_clean_svn		= $(CD) $1 && $(RM) `svn st | awk '{ if ($$1 == "?") print $$2 }'` && svn revert -R .

repo_clean			= $(call repo_clean_$(REPO_TYPE), $1)
