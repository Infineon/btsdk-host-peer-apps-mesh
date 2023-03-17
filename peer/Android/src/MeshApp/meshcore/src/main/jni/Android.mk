#
# Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
# an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
#
# This software, including source code, documentation and related
# materials ("Software") is owned by Cypress Semiconductor Corporation
# or one of its affiliates ("Cypress") and is protected by and subject to
# worldwide patent protection (United States and foreign),
# United States copyright laws and international treaty provisions.
# Therefore, you may use this Software only as provided in the license
# agreement accompanying the software package from which you
# obtained this Software ("EULA").
# If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
# non-transferable license to copy, modify, and compile the Software
# source code solely for use in connection with Cypress's
# integrated circuit products.  Any reproduction, modification, translation,
# compilation, or representation of this Software except as specified
# above is prohibited without the express written permission of Cypress.
#
# Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
# reserves the right to make changes to the Software without notice. Cypress
# does not assume any liability arising out of the application or use of the
# Software or any product or circuit described in the Software. Cypress does
# not authorize its products for use in any products where a malfunction or
# failure of the Cypress product may reasonably be expected to result in
# significant property damage, injury or death ("High Risk Product"). By
# including Cypress's product in a High Risk Product, the manufacturer
# of such system or application assumes all risk of such use and in doing
# so agrees to indemnify Cypress against all liability.
#
LOCAL_PATH := $(call my-dir)

MESH_DFU_SUPPORT := FALSE

DEV_KIT_PATH         := $(LOCAL_PATH)/../../../../../../../../../../dev-kit
MESH_CLIENT_LIB_PATH := $(LOCAL_PATH)/../../../../../../../../mesh_client_lib
BASELIB_PATH         := $(DEV_KIT_PATH)/baselib/20819A1/COMPONENT_20819A1
APP_INCLUDE_PATH     := $(LOCAL_PATH)/../../../../../../../../include

include $(CLEAR_VARS)
LOCAL_MODULE    := ccStaticLibrary
LOCAL_SRC_FILES := prebuild/$(TARGET_ARCH_ABI)/libwicedmesh.a
LOCAL_C_INCLUDES := $(wildcard $(DEV_KIT_PATH)/btsdk-include)
LOCAL_C_INCLUDES += $(wildcard $(DEV_KIT_PATH)/libraries/mesh_libs/include)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/hal)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/internal)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/stack)
# LOCAL_C_INCLUDES += $(wildcard $(APP_INCLUDE_PATH))
LOCAL_C_INCLUDES += $(wildcard $(MESH_CLIENT_LIB_PATH))
LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH)/mesh_libs)
LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH))
LOCAL_CFLAGS += -fno-stack-protector
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := native-lib
MY_CPP_LIST := $(wildcard $(LOCAL_PATH)/native-lib.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_app.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/mesh_main.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/aes.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/aes_cmac.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/ccm.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/p_256_ecc_pp.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/p_256_curvepara.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/p_256_multprecision.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/sha2.c)
MY_CPP_LIST += $(wildcard $(MESH_CLIENT_LIB_PATH)/meshdb.c)
MY_CPP_LIST += $(wildcard $(MESH_CLIENT_LIB_PATH)/wiced_bt_mesh_db.c)
MY_CPP_LIST += $(wildcard $(MESH_CLIENT_LIB_PATH)/wiced_mesh_client.c)
ifeq ($(MESH_DFU_SUPPORT), TRUE)
LOCAL_CFLAGS += -DMESH_DFU_ENABLED
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/mesh_libs/mesh_fw_provider.c)
MY_CPP_LIST += $(wildcard $(MESH_CLIENT_LIB_PATH)/wiced_mesh_client_dfu.c)
endif

LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_STATIC_LIBRARIES := ccStaticLibrary
LOCAL_CFLAGS += -fno-stack-protector
LOCAL_LDLIBS := -llog
LOCAL_C_INCLUDES := $(wildcard $(DEV_KIT_PATH)/btsdk-include)
LOCAL_C_INCLUDES += $(wildcard $(DEV_KIT_PATH)/libraries/mesh_libs/include)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/hal)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/internal)
LOCAL_C_INCLUDES += $(wildcard $(BASELIB_PATH)/include/stack)
# LOCAL_C_INCLUDES += $(wildcard $(APP_INCLUDE_PATH))
LOCAL_C_INCLUDES += $(wildcard $(MESH_CLIENT_LIB_PATH))
LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH)/mesh_libs)
LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH))
include $(BUILD_SHARED_LIBRARY)

APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
