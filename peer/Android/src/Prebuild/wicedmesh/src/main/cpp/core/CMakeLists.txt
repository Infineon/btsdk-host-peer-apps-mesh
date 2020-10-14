cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

add_definitions(-DMESH_OVER_GATT_ONLY)
add_definitions(-DREMOTE_PROVISIONG_SERVER_SUPPORTED)
add_definitions(-DWICED_BT_MESH_TRACE_ENABLE)
add_definitions(-DWICED_BT_TRACE_ENABLE)
add_definitions(-DMESH_MODELS_DEBUG)
add_definitions(-DMESH_CORE_DEBUG)
add_definitions(-DPB_GATT_SUPPORTED)
add_definitions(-DMESH_DFU_SUPPORTED)

set(INCLUDE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/baselib/20819A1/include/hal/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/baselib/20819A1/include/internal"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/baselib/20819A1/include/stack/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/baselib/20819A1/include/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/mesh_libs/"
 )

file(GLOB MY_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/access_layer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/aes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/brg_endian.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/brg_types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/bt_target_.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/core_aes_ccm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/ecdh.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/fid_app.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_cfg_model.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_crt_msg.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_heartbeat.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_identity.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_msg_handler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/friend.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/friendship.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/health.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/key_refresh.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/lower_transport_layer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/low_power.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/mesh_core.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/mesh_trace.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/network_layer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/pb_transport.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/provisioning_int.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/p_256_ecc_pp.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/p_256_multprecision.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/p_256_types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/upper_transport_layer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/access_layer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/core_aes_ccm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/ecdh.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_cfg_model.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_crt_msg.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_heartbeat.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_identity.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/foundation_msg_handler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/friend.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/friendship.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/health.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/key_refresh.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/lower_transport_layer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/low_power.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/mesh_core.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/mesh_util.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/mesh_event.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/network_layer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/pb_transport.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/provisioning.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/iv_updt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/upper_transport_layer.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/directed_forwarding.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/directed_forwarding_ctrl_msg.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/directed_forwarding_timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_core_lib/directed_forwarding_msg.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_default_transition_time_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_level_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_onoff_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_power_onoff_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/light_ctl_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/light_lc_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/light_hsl_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/light_lightness_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/mesh_model_utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/mesh_model_utils.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_property_client.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/generic_property_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/fw_distributor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/fw_update_server.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/object_transfer_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/object_transfer_server.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/mesh_gatt_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/remote_provision_server.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/sensor_server.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/sensor_client.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/blob_transfer_client.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/blob_transfer_server.c"
#	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_models_lib/vendor.c"
#	"${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/gatt_utils_lib/gatt_utils_lib.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_config_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_health_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_provisioner.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_proxy_client.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_core_defs.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/libraries/internal/mesh_provisioner_lib/mesh_fw_provider.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/20819/hal/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/20819/stack/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/20819/internal/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/20819/*.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../dev-kit/btsdk-include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/wiced_bt_ble.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../../../../../../../../include/wiced_bt_mesh_core.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mesh_libs/*.h"
)

add_library(wicedmesh STATIC
"${MY_SRC}"
)

target_include_directories(wicedmesh PUBLIC
 ${INCLUDE_DIR})

# copy out the lib binary... need to leave the static lib around to pass gradle check
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set_target_properties(wicedmesh
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/wicedmesh/prebuild/${ANDROID_ABI}"
                      )
