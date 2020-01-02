LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

$(warning "abi: $(TARGET_ARCH_ABI)")

ifeq "$(TARGET_ARCH_ABI)" "arm64-v8a"

#LOCAL_CXXFLAGS +=  -g -O0
#LOCAL_ARM_MODE := arm
LOCAL_MODULE    		:= imodel
LOCAL_STATIC_LIBRARIES	:= dump_with_ret dump replace dump_just_ret
LOCAL_C_INCLUDES 		:= $(LOCAL_PATH)/../../include
LOCAL_SRC_FILES 		:= iModel.cpp
LOCAL_LDLIBS 			+= -L$(SYSROOT)/usr/lib -llog

include $(BUILD_STATIC_LIBRARY)

endif