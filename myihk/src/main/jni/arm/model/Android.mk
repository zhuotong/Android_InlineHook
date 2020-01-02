LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

$(warning "abi: $(TARGET_ARCH_ABI)")

#ifeq "$(TARGET_ARCH_ABI)" "armeabi-v7a"
ifneq ($(findstring $(TARGET_ARCH_ABI), "armeabi-v7a"  "armeabi"),)


#LOCAL_CXXFLAGS +=  -g -O0
#LOCAL_ARM_MODE := arm
LOCAL_MODULE    		:= imodel
LOCAL_STATIC_LIBRARIES	:= dump dump_just_ret dump_with_ret replace
LOCAL_C_INCLUDES 		:= $(LOCAL_PATH)/../../include
LOCAL_SRC_FILES 		:= iModel.cpp
LOCAL_LDLIBS 			+= -L$(SYSROOT)/usr/lib -llog

include $(BUILD_STATIC_LIBRARY)

endif