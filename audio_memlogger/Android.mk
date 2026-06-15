LOCAL_PATH := $(call my-dir)

# === Header-only module ===
include $(CLEAR_VARS)

LOCAL_MODULE := libarmemlog_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/armemlog/inc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := HEADER_LIBRARIES
LOCAL_MODULE_OWNER := qti
LOCAL_VENDOR_MODULE := true

include $(BUILD_HEADER_LIBRARY)


# === Shared library module ===
include $(CLEAR_VARS)

LOCAL_MODULE := libarmemlog
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES := armemlog/src/mem_logger.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libexpat \
    liblx-osal

LOCAL_HEADER_LIBRARIES := libarmemlog_headers

include $(BUILD_SHARED_LIBRARY)
