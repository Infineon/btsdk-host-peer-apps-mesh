cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(DEV_KIT_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit)

add_definitions(-DMESH_OVER_GATT_ONLY)
add_definitions(-DREMOTE_PROVISIONG_SERVER_SUPPORTED)
add_definitions(-DWICED_BT_MESH_TRACE_ENABLE)
add_definitions(-DWICED_BT_TRACE_ENABLE)
add_definitions(-DMESH_MODELS_DEBUG)
add_definitions(-DMESH_CORE_DEBUG)
add_definitions(-DPB_GATT_SUPPORTED)
add_definitions(-DMESH_DFU_SUPPORTED)

set(INCLUDE_DIR
  "${DEV_KIT_PATH}/btsdk-include/"
  "${DEV_KIT_PATH}/libraries/mesh_libs/include/"
  "${DEV_KIT_PATH}/baselib/20819A1/COMPONENT_20819A1/include/"
  "${DEV_KIT_PATH}/baselib/20819A1/COMPONENT_20819A1/include/hal/"
  "${DEV_KIT_PATH}/baselib/20819A1/COMPONENT_20819A1/include/internal"
  "${DEV_KIT_PATH}/baselib/20819A1/COMPONENT_20819A1/include/stack/"
  "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/"
  "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/"
  "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_provisioner_lib/"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mesh_libs/"
 )

file(GLOB MY_SRC
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/access_layer.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/core_aes_ccm.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/directed_forwarding.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/directed_forwarding_ctrl_msg.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/directed_forwarding_msg.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/directed_forwarding_timer.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/ecdh.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation_cfg_model.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation_crt_msg.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation_heartbeat.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation_identity.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/foundation_msg_handler.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/friend.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/friendship.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/health.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/iv_updt.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/key_refresh.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/lower_transport_layer.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/low_power.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/mesh_core.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/mesh_event.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/mesh_util.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/network_layer.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/pb_transport.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/provisioning.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_core_lib/upper_transport_layer.c"

    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/blob_transfer_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/blob_transfer_server.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/fw_distributor.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/fw_update_server.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/generic_default_transition_time_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/generic_level_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/generic_onoff_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/generic_power_onoff_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/generic_property_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/light_ctl_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/light_hsl_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/light_lc_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/light_xyl_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/light_lightness_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/mesh_gatt_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/mesh_model_utils.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/object_transfer_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/object_transfer_server.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/remote_provision_server.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/scene_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/sensor_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_models_lib/sensor_server.c"

    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_provisioner_lib/mesh_config_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_provisioner_lib/mesh_health_client.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_provisioner_lib/mesh_provisioner.c"
    "${DEV_KIT_PATH}/libraries/mesh_libs/mesh_provisioner_lib/mesh_proxy_client.c"
)

add_library(wicedmesh STATIC "${MY_SRC}")

target_include_directories(wicedmesh PUBLIC ${INCLUDE_DIR})

# copy out the lib binary... need to leave the static lib around to pass gradle check
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set_target_properties(wicedmesh
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/wicedmesh/prebuild/${ANDROID_ABI}"
                      )
