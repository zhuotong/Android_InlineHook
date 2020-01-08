LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

$(warning "abi: $(TARGET_ARCH_ABI)")

ifeq "$(TARGET_ARCH_ABI)" "arm64-v8a"

#LOCAL_CXXFLAGS +=  -g -O0
#LOCAL_ARM_MODE := arm
LOCAL_MODULE    			:= dump_with_ret
LOCAL_C_INCLUDES 			:= $(LOCAL_PATH)/../../../include $(LOCAL_PATH)/../../asm
LOCAL_SRC_FILES 			:= ../mhk.c dump.c lr.cpp ../fixPCOpcode.c ../../asm/dump_neon_with_ret.s


include $(BUILD_STATIC_LIBRARY)

endif