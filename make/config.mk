################################################################################                                                    
## @file:   config.mk
## @author  张框正
## @brief   目前供publish使用
## @version 1.0
###############################################################################
Publish_list := test private public
PublishType := $(type)

ifeq ($(PublishType),)
    PublishType := test
else
   ifeq ($(findstring $(PublishType),$(Publish_list)),)
       PublishType := test
   endif	
endif
