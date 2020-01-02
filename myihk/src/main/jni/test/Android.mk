LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

$(warning "abi: $(TARGET_ARCH_ABI)")

#ifeq "$(TARGET_ARCH_ABI)" "arm64-v8a"

#LOCAL_CXXFLAGS +=  -g -O0
#LOCAL_ARM_MODE := arm
LOCAL_MODULE    		:= shk
LOCAL_STATIC_LIBRARIES	:= imodel #dump_with_ret dump replace
LOCAL_C_INCLUDES 		:= $(LOCAL_PATH)/../include
LOCAL_SRC_FILES 		:= main.cpp
#LOCAL_SRC_FILES 		:= main_exportHook.cpp
#LOCAL_SRC_FILES 		:= main_arm.cpp
LOCAL_LDLIBS 			+= -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)

#endif