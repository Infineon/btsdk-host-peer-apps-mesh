/*
 * Copyright Cypress Semiconductor
 */

/** @file
 *
 * This file implements the MeshNativeHelper class which wraps all mesh libraries and fucntions.
 */

#import "stdio.h"
#import "stdlib.h"
#import "time.h"
#import "MeshNativeHelper.h"
#import "IMeshNativeCallback.h"
#import "mesh_main.h"
#import "wiced_timer.h"
#import "wiced_bt_ble.h"
#import "wiced_bt_mesh_model_defs.h"
#import "wiced_bt_mesh_models.h"
#import "wiced_bt_mesh_event.h"
#import "wiced_bt_mesh_core.h"
#import "wiced_bt_mesh_provision.h"
#import "wiced_bt_mesh_db.h"
#import "wiced_mesh_client.h"
#ifdef MESH_DFU_ENABLED
#import "wiced_mesh_client_dfu.h"
#import "wiced_bt_mesh_dfu.h"
#endif
#import <CommonCrypto/CommonDigest.h>

extern void mesh_application_init(void);
extern void mesh_client_advert_report(uint8_t *bd_addr, uint8_t addr_type, int8_t rssi, uint8_t *adv_data);

@implementation MeshNativeHelper
{
    // define instance variables.
}


// define class variables.
static id nativeCallbackDelegate;   // Object instance that obeys IMeshNativeHelper protocol
static MeshNativeHelper *_instance;
static char provisioner_uuid[40];  // Used to store provisioner UUID string.
static dispatch_once_t onceToken;
static dispatch_once_t zoneOnceToken;

typedef struct mesh_device_config_params
{
    int is_gatt_proxy;
    int is_friend;
    int is_relay;
    int send_net_beacon;
    int relay_xmit_count;
    int relay_xmit_interval;
    int default_ttl;
    int net_xmit_count;
    int net_xmit_interval;
    int publish_credential_flag;       ///< Value of the Friendship Credential Flag
    int publish_ttl;                   ///< Default TTL value for the outgoing messages
    int publish_retransmit_count;      ///< Number of retransmissions for each published message
    int publish_retransmit_interval;   ///< Interval in milliseconds between retransmissions
} mesh_device_config_params_t;
static mesh_device_config_params_t mesh_device_config_params = {
    MESH_DEVICE_DEFAULT_CONFIG_IS_GATT_PROXY,
    MESH_DEVICE_DEFAULT_CONFIG_IS_FRIEND,
    MESH_DEVICE_DEFAULT_CONFIG_IS_RELAY,
    MESH_DEVICE_DEFAULT_CONFIG_SEND_NET_BEACON,
    MESH_DEVICE_DEFAULT_CONFIG_RELAY_XMIT_COUNT,
    MESH_DEVICE_DEFAULT_CONFIG_RELAY_XMIT_INTERNAL,
    MESH_DEVICE_DEFAULT_CONFIG_DEFAULT_TTL,
    MESH_DEVICE_DEFAULT_CONFIG_NET_XMIT_COUNT,
    MESH_DEVICE_DEFAULT_CONFIG_NET_XMIT_INTERNVAL,
    MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_CREDENTIAL_FLAG,
    MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_TTL,
    MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_RETRANSMIT_COUNT,
    MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_RETRANSMIT_INTERVAL
};
#define MESH_DEVICE_CONFIG_PARAMS_FILE_NAME "NetParameters.bin"

#ifdef MESH_DFU_ENABLED
// siglone instance of DFW metadata data.
static uint8_t dfuFwId[WICED_BT_MESH_MAX_FIRMWARE_ID_LEN];
static uint32_t dfuFwIdLen;
static uint8_t dfuMetadataData[WICED_BT_MESH_MAX_METADATA_LEN];
static uint32_t dfuMetadataDataLen;
void mesh_dfu_metadata_init()
{
    dfuFwIdLen = 0;
    memset(dfuFwId, 0, WICED_BT_MESH_MAX_FIRMWARE_ID_LEN);
    dfuFwIdLen = 0;
    memset(dfuMetadataData, 0, WICED_BT_MESH_MAX_METADATA_LEN);
}
#endif  // #ifdef MESH_DFU_ENABLED

/*
 * Implementation of APIs that required by wiced mesh core stack library for event and data callback.
 */

void meshClientUnprovisionedDeviceFoundCb(uint8_t *uuid, uint16_t oob, uint8_t *name, uint8_t name_len)
{
    NSString *deviceName = nil;
    if (uuid == NULL) {
        WICED_BT_TRACE("[MeshNativeHelper meshClientFoundUnprovisionedDeviceCb] error: invalid parameters, uuid=0x%p, name_len=%d\n", uuid, name_len);
        return;
    }
    if (name != NULL && name_len > 0) {
        deviceName = [[NSString alloc] initWithBytes:name length:name_len encoding:NSUTF8StringEncoding];
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientFoundUnprovisionedDeviceCb] found device name: %s, oob: 0x%04x, uuid: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", deviceName.UTF8String, oob, uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    [nativeCallbackDelegate onDeviceFound:[[NSUUID alloc] initWithUUIDBytes:uuid]
                                      oob:oob
                                  uriHash:0
                                     name:deviceName];
}

void meshClientProvisionCompleted(uint8_t status, uint8_t *p_uuid)
{
    if (p_uuid == NULL) {
        WICED_BT_TRACE("[MeshNativeHelper meshClientProvisionCompleted] error: invalid parameters, p_uuid=0x%p\n", p_uuid);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientProvisionCompleted] status: %u\n", status);
    [nativeCallbackDelegate meshClientProvisionCompletedCb:status uuid:[[NSUUID alloc] initWithUUIDBytes:p_uuid]];
}

// called when mesh network connection status changed.
void linkStatus(uint8_t is_connected, uint32_t connId, uint16_t addr, uint8_t is_over_gatt)
{
    WICED_BT_TRACE("[MeshNativeHelper linkStatus] is_connected: %u, connId: 0x%08x, addr: 0x%04x, is_over_gatt: %u\n", is_connected, connId, addr, is_over_gatt);
    [nativeCallbackDelegate onLinkStatus:is_connected connId:connId addr:addr isOverGatt:is_over_gatt];
}

// callback for meshClientConnect API.
void meshClientNodeConnectionState(uint8_t status, char *p_name)
{
    if (p_name == NULL || *p_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientNodeConnectionState] error: invalid parameters, p_name=0x%p\n", p_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientNodeConnectionState] status: 0x%02x, device_name: %s\n", status, p_name);
    [nativeCallbackDelegate meshClientNodeConnectStateCb:status componentName:[NSString stringWithUTF8String:(const char *)p_name]];
}

void resetStatus(uint8_t status, char *device_name)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper resetStatus] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper resetStatus] status: 0x%02x, device_name: %s\n", status, device_name);
    [nativeCallbackDelegate onResetStatus:status devName:[NSString stringWithUTF8String:(const char *)device_name]];
}

void meshClientOnOffState(const char *device_name, uint8_t target, uint8_t present, uint32_t remaining_time)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientOnOffState] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientOnOffState] device_name: %s, target: %u, present: %u, remaining_time: %u\n", device_name, target, present, remaining_time);
    [nativeCallbackDelegate meshClientOnOffStateCb:[NSString stringWithUTF8String:(const char *)device_name] target:target present:present remainingTime:remaining_time];
}

void meshClientLevelState(const char *device_name, int16_t target, int16_t present, uint32_t remaining_time)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientLevelState] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientLevelState] device_name: %s, target: %u, present: %u, remaining_time: %u\n", device_name, target, present, remaining_time);
    [nativeCallbackDelegate meshClientLevelStateCb:[NSString stringWithUTF8String:(const char *)device_name]
                                            target:target
                                           present:present
                                    remainingTime:remaining_time];
}

void meshClientLightnessState(const char *device_name, uint16_t target, uint16_t present, uint32_t remaining_time)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientLightnessState] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientLightnessState] device_name: %s, target: %u, present: %u, remaining_time: %u\n", device_name, target, present, remaining_time);
    [nativeCallbackDelegate meshClientLightnessStateCb:[NSString stringWithUTF8String:(const char *)device_name]
                                                target:target
                                               present:present
                                         remainingTime:remaining_time];
}

void meshClientHslState(const char *device_name, uint16_t lightness, uint16_t hue, uint16_t saturation, uint32_t remaining_time)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientHslState] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientHslState] device_name: %s, lightness: %u, hue: %u, saturation: %u, remaining_time: %u\n",
          device_name, lightness, hue, saturation, remaining_time);
    [nativeCallbackDelegate meshClientHslStateCb:[NSString stringWithUTF8String:(const char *)device_name]
                                       lightness:lightness hue:hue saturation:saturation remainingTime:remaining_time];
}

void meshClientCtlState(const char *device_name, uint16_t present_lightness, uint16_t present_temperature, uint16_t target_lightness, uint16_t target_temperature, uint32_t remaining_time)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientCtlState] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientCtlState] device_name: %s, present_lightness: %u, present_temperature: %u, target_lightness: %u, target_temperature: %u, remaining_time: %u\n", device_name, present_lightness, present_temperature, target_lightness, target_temperature, remaining_time);
    [nativeCallbackDelegate meshClientCtlStateCb:[NSString stringWithUTF8String:(const char *)device_name]
                                presentLightness:present_lightness
                              presentTemperature:present_temperature
                                 targetLightness:target_lightness
                               targetTemperature:target_temperature
                                   remainingTime:remaining_time];
}

void meshClientDbChangedState(char *mesh_name)
{
    if (mesh_name == NULL || *mesh_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientDbChangedState] error: invalid parameters, mesh_name=0x%p\n", mesh_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientDbChangedState] mesh_name: %s\n", mesh_name);
    [nativeCallbackDelegate onDatabaseChangedCb:[NSString stringWithUTF8String:mesh_name]];
}

void meshClientSensorStatusChangedCb(const char *device_name, int property_id, uint8_t length, uint8_t *value)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientSensorStatusChangedCb] error: invalid device_name:%s or property_id:%d, length=%d\n", device_name, property_id, length);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorStatusChangedCb] device_name:%s, property_id:%d, value lenght:%d\n", device_name, property_id, length);
    [nativeCallbackDelegate onMeshClientSensorStatusChanged:[NSString stringWithUTF8String:device_name]
                                                 propertyId:(uint32_t)property_id
                                                       data:[NSData dataWithBytes:value length:length]];
}

void meshClientVendorSpecificDataCb(const char *device_name, uint16_t company_id, uint16_t model_id, uint8_t opcode, uint8_t ttl, uint8_t *p_data, uint16_t data_len)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientVendorSpecificDataCb] error: invalid device_name NULL\n");
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientVendorSpecificDataCb] device_name:%s, company_id:%d, model_id:%d, opcode:%d, data_len:%d\n",
          device_name, company_id, model_id, opcode, data_len);
    [nativeCallbackDelegate onMeshClientVendorSpecificDataChanged:[NSString stringWithUTF8String:device_name]
                                                        companyId:company_id
                                                          modelId:model_id
                                                           opcode:opcode
                                                              ttl:ttl
                                                             data:[NSData dataWithBytes:p_data length:data_len]];
}

mesh_client_init_t mesh_client_init_callback = {
    .unprovisioned_device_callback = meshClientUnprovisionedDeviceFoundCb,
    .provision_status_callback = meshClientProvisionCompleted,
    .connect_status_callback = linkStatus,
    .node_connect_status_callback = meshClientNodeConnectionState,
    .database_changed_callback = meshClientDbChangedState,
    .on_off_changed_callback = meshClientOnOffState,
    .level_changed_callback = meshClientLevelState,
    .lightness_changed_callback = meshClientLightnessState,
    .hsl_changed_callback = meshClientHslState,
    .ctl_changed_callback = meshClientCtlState,
    .sensor_changed_callback = meshClientSensorStatusChangedCb,
    .vendor_specific_data_callback = meshClientVendorSpecificDataCb,
};

// timer based iOS platform.
static uint32_t gMeshTimerId = 1;       // always > 0; 1 is the first and the app whole life second pediodic timer; other values can be reused.
static NSMutableDictionary *gMeshTimersDict = nil;

-(void) meshTimerInit
{
    if (gMeshTimersDict == nil) {
        EnterCriticalSection();
        if (gMeshTimersDict == nil) {
            gMeshTimersDict = [[NSMutableDictionary alloc] initWithCapacity:5];
        }
        LeaveCriticalSection();
    }
}

-(uint32_t) allocateMeshTimerId
{
    uint32_t timerId = 0;   // default set to invalid timerId
    EnterCriticalSection();
    do {
        timerId = gMeshTimerId++;
        if (timerId == 0) { // the gMeshTimerid must be round back.
            continue;
        }
        // avoid the same timerId was used, because some timer running for long time or  presistent.
        if ([gMeshTimersDict valueForKey:[NSString stringWithFormat:@"%u", timerId]] == nil) {
            break;  // return new not used timerId.
        }
    } while (true);
    LeaveCriticalSection();
    return timerId;
}

-(void) timerFiredMethod:(NSTimer *)timer
{
    NSString * timerKey = timer.userInfo;
    uint32_t timerId = 0;
    if (timerKey != nil) {
        timerId = (uint32_t)timerKey.longLongValue;
    }

    //WICED_BT_TRACE("[MeshNativeHelper timerFiredMethod] timerId:%u\n", timerId);
    MeshTimerFunc((long)timerId);
}

/*
 * @param timeout   Timer trigger interval, uint: milliseconds.
 * @param type      The timer type.
 *                  When the timer type is WICED_SECONDS_TIMER or WICED_MILLI_SECONDS_TIMER,
 *                      the timer will be invalidated after it fires.
 *                  When the timer type is WICED_SECONDS_PERIODIC_TIMER or WICED_MILLI_SECONDS_PERIODIC_TIMER,
 *                      the timer will repeatedly reschedule itself until stopped.
 * @return          A non-zero timerId will be returned when started on success. Otherwize, 0 will be return on failure.
 */
-(uint32_t) meshStartTimer:(uint32_t)timeout type:(uint16_t)type
{
    [MeshNativeHelper.getSharedInstance meshTimerInit];
    uint32_t timerId = [MeshNativeHelper.getSharedInstance allocateMeshTimerId];
    Boolean repeats = (type == WICED_SECONDS_PERIODIC_TIMER || type == WICED_MILLI_SECONDS_PERIODIC_TIMER) ? true : false;
    NSTimeInterval interval = (NSTimeInterval)timeout;
    interval /= (NSTimeInterval)1000;
    NSString * timerKey = [NSString stringWithFormat:@"%u", timerId];
    NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                                      target:MeshNativeHelper.getSharedInstance
                                                    selector:@selector(timerFiredMethod:)
                                                    userInfo:timerKey
                                                     repeats:repeats];
    if (timer == nil) {
        WICED_BT_TRACE("[MeshNativeHelper meshStartTimer] error: failed to create and init the timer\n");
        return 0;
    }

    NSArray *timerInfo = [[NSArray alloc] initWithObjects:[NSNumber numberWithUnsignedInt:timerId], [NSNumber numberWithUnsignedShort:type], timer, nil];
    [gMeshTimersDict setObject:timerInfo forKey:timerKey];
    //WICED_BT_TRACE("[MeshNativeHelper meshStartTimer] timerId:%u started, type=%u, interval=%f\n", timerId, type, interval);
    return timerId;
}
uint32_t start_timer(uint32_t timeout, uint16_t type) {
    return [MeshNativeHelper.getSharedInstance meshStartTimer:timeout type:type];
}

-(void) meshStopTimer:(uint32_t)timerId
{
    [MeshNativeHelper.getSharedInstance meshTimerInit];
    NSString *timerKey = [NSString stringWithFormat:@"%u", timerId];
    NSArray *timerInfo = [gMeshTimersDict valueForKey:timerKey];
    if (timerInfo != nil && [timerInfo count] == 3) {
        NSTimer *timer = timerInfo[2];
        if (timer != nil) {
            [timer invalidate];
        }
    }
    [gMeshTimersDict removeObjectForKey:timerKey];
    //WICED_BT_TRACE("[MeshNativeHelper meshStopTimer] timerId:%u stopped\n", timerId);
}
void stop_timer(uint32_t timerId)
{
    [MeshNativeHelper.getSharedInstance meshStopTimer:timerId];
}

/*
 * @param timeout   Timer trigger interval, uint: milliseconds.
 * @param type      The timer type.
 *                  When the timer type is WICED_SECONDS_TIMER or WICED_MILLI_SECONDS_TIMER,
 *                      the timer will be invalidated after it fires.
 *                  When the timer type is WICED_SECONDS_PERIODIC_TIMER or WICED_MILLI_SECONDS_PERIODIC_TIMER,
 *                      the timer will repeatedly reschedule itself until stopped.
 * @return          The same non-zero timerId will be returned when restarted on success. Otherwize, 0 will be return on failure.
 */
-(uint32_t) meshRestartTimer:(uint32_t)timeout timerId:(uint32_t)timerId
{
    [MeshNativeHelper.getSharedInstance meshTimerInit];
    NSString *timerKey = [NSString stringWithFormat:@"%u", timerId];
    NSArray *timerInfo = [gMeshTimersDict valueForKey:timerKey];
    NSNumber *numType = timerInfo[1];
    NSTimer *timer = timerInfo[2];
    uint16_t type;

    if (timerInfo == nil || [timerInfo count] != 3 || timer == nil || numType == nil) {
        WICED_BT_TRACE("[MeshNativeHelper meshRestartTimer] error: failed to fetch the timer with timerId=%u\n", timerId);
        return 0;
    }

    type = [numType unsignedShortValue];
    [timer invalidate];

    Boolean repeats = (type == WICED_SECONDS_PERIODIC_TIMER || type == WICED_MILLI_SECONDS_PERIODIC_TIMER) ? true : false;
    NSTimeInterval interval = (NSTimeInterval)timeout;
    interval /= (NSTimeInterval)1000;
    timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                             target:MeshNativeHelper.getSharedInstance
                                           selector:@selector(timerFiredMethod:)
                                           userInfo:timerKey
                                            repeats:repeats];
    if (timer == nil) {
        WICED_BT_TRACE("[MeshNativeHelper meshRestartTimer] error: failed to create and init the timer\n");
        return 0;
    }

    timerInfo = [[NSArray alloc] initWithObjects:[NSNumber numberWithUnsignedInt:timerId], [NSNumber numberWithUnsignedShort:type], timer, nil];
    [gMeshTimersDict setObject:timerInfo forKey:timerKey];
    //WICED_BT_TRACE("[MeshNativeHelper meshRestartTimer] timerId:%u, type=%u, interval=%f\n", timerId, type, interval);
    return timerId;
}
uint32_t restart_timer(uint32_t timeout, uint32_t timerId ) {
    return [MeshNativeHelper.getSharedInstance meshRestartTimer:timeout timerId:timerId];
}

void mesh_provision_gatt_send(uint16_t connId, uint8_t *packet, uint32_t packet_len)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_provision_gatt_send] connId=%d, packet_len=%d\n", connId, packet_len);
    if (packet == NULL || packet_len == 0) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_provision_gatt_send] error: connId=%d packet=0x%p, packet_len=%u\n", connId, packet, packet_len);
        return;
    }
    NSData *data = [[NSData alloc] initWithBytes:packet length:packet_len];
    [nativeCallbackDelegate onProvGattPktReceivedCallback:connId data:data];
}

void proxy_gatt_send_cb(uint32_t connId, uint32_t ref_data, const uint8_t *packet, uint32_t packet_len)
{
    WICED_BT_TRACE("[MeshNativeHelper proxy_gatt_send_cb] connId=%d, packet_len=%d\n", connId, packet_len);
    if (packet == NULL || packet_len == 0) {
        WICED_BT_TRACE("[MeshNativeHelper proxy_gatt_send_cb] error: invalid parameters, packet=0x%p, packet_len=%u\n", packet, packet_len);
        return;
    }
    NSData *data = [[NSData alloc] initWithBytes:packet length:packet_len];
    [nativeCallbackDelegate onProxyGattPktReceivedCallback:connId data:data];
}

wiced_bool_t mesh_bt_gatt_le_disconnect(uint32_t connId)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_bt_gatt_le_disconnect] connId=%d\n", connId);
    return [nativeCallbackDelegate meshClientDisconnect:(uint16_t)connId];
}

wiced_bool_t mesh_bt_gatt_le_connect(wiced_bt_device_address_t bd_addr, wiced_bt_ble_address_type_t bd_addr_type,
                                     wiced_bt_ble_conn_mode_t conn_mode, wiced_bool_t is_direct)
{
    if (bd_addr == NULL) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_bt_gatt_le_connect] invalid parameters, bd_addr=0x%p\n", bd_addr);
        return false;
    }
    NSData *bdAddr = [[NSData alloc] initWithBytes:bd_addr length:BD_ADDR_LEN];
    WICED_BT_TRACE("[MeshNativeHelper mesh_bt_gatt_le_connect] bd_addr=%02x %02x %02x %02x %02x %02x, bd_addr_type=%d, conn_mode=%d, is_direct=%d\n",
                   bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5],
                   bd_addr_type, conn_mode, is_direct);
    return [nativeCallbackDelegate meshClientConnect:bdAddr];
}

wiced_bool_t mesh_set_scan_type(uint8_t is_active)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_set_scan_type] is_active=%d\n", is_active);
    return [nativeCallbackDelegate meshClientSetScanTypeCb:is_active];
}

wiced_bool_t mesh_adv_scan_start(void)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_adv_scan_start]\n");
    return [nativeCallbackDelegate meshClientAdvScanStartCb];
}

void mesh_adv_scan_stop(void)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_adv_scan_stop]\n");
    [nativeCallbackDelegate meshClientAdvScanStopCb];
}

/*
 * Only one instance of the MeshNativeHelper class can be created all the time.
 */
+(MeshNativeHelper *) getSharedInstance
{
    dispatch_once(&onceToken, ^{
        _instance = [[self alloc] init];
    });
    return _instance;
}

+(instancetype) allocWithZone:(struct _NSZone *)zone
{
    //static dispatch_once_t onceToken;
    dispatch_once(&zoneOnceToken, ^{
        _instance = [super allocWithZone:zone];
        [_instance instanceInit];
    });
    return _instance;
}

-(id)copyWithZone:(NSZone *)zone {
    return _instance;
}

/* Do all necessory initializations for the shared class instance. */
-(void) instanceInit
{
    [self meshTimerInit];
    [self meshBdAddrDictInit];
#ifdef MESH_DFU_ENABLED
    mesh_dfu_metadata_init();
#endif
}

+(NSString *) getProvisionerUuidFileName
{
    return @"prov_uuid.bin";
}

+(int) setFileStorageAtPath:(NSString *)path
{
    return [MeshNativeHelper setFileStorageAtPath:path provisionerUuid:nil];
}

+(int) setFileStorageAtPath:(NSString *)path provisionerUuid: (NSUUID *)provisionerUuid
{
    Boolean bRet = true;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    if (![fileManager fileExistsAtPath:path]) {
        bRet = [fileManager createDirectoryAtPath:path withIntermediateDirectories:true attributes:nil error:nil];
        WICED_BT_TRACE("[MeshNativeHelper setFileStorageAtPath] create direcotry \"%s\" %s\n", path.UTF8String, bRet ? "success" : "failed");
    }
    if (!bRet || ![fileManager isWritableFileAtPath:path]) {
        WICED_BT_TRACE("[MeshNativeHelper setFileStorageAtPath] error: cannot wirte at path:\"%s\", bRet=%u\n", path.UTF8String, bRet);
        return -1;
    }
    // set this file directory to be current working directory
    const char *cwd = [path cStringUsingEncoding:NSASCIIStringEncoding];
    int cwdStatus = chdir(cwd);
    if (cwdStatus != 0) {
        WICED_BT_TRACE("[MeshNativeHelper setFileStorageAtPath] error: unable to change current working directory to \"%s\" \n", cwd);
        return -2;
    } else {
        WICED_BT_TRACE("[MeshNativeHelper setFileStorageAtPath] Done, change current working directory to \"%s\"\n", path.UTF8String);
    }

    // try to create the mesh library log file if not existing, then open it for logging.
    set_log_file_path((char *)cwd);
    open_log_file();

    return [MeshNativeHelper updateProvisionerUuid:nil];
}

/**
 * This function generates version 4 UUID (Random) per rfc4122:
 * - Set the two most significant bits(bits 6 and 7) of the
 *   clock_seq_hi_and_reserved to zero and one, respectively.
 * - Set the four most significant bits(bits 12 through 15) of the
 *   time_hi_and_version field to the 4 - bit version number.
 * - Set all the other bits to randomly(or pseudo - randomly) chosen values.
 */
+(NSUUID *) generateRfcUuid
{
    unsigned char rfcuuid[16];
    [NSUUID.UUID getUUIDBytes:rfcuuid];

    // The version field is 4.
    rfcuuid[6] = (rfcuuid[6] & 0x0f) | 0x40;
    // The variant field is 10B
    rfcuuid[8] = (rfcuuid[8] & 0x3f) | 0x80;
    return [[NSUUID alloc] initWithUUIDBytes:rfcuuid];
}

+(int) updateProvisionerUuid:(NSUUID *)provisionerUuid
{
    Boolean bRet = true;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    char path[PATH_MAX];
    getcwd(path, PATH_MAX);
    WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] current working path: %s\n", path);

    // check if the provisioner_uuid has been read from or has been created and written to the storage file.
    if ([fileManager fileExistsAtPath:MeshNativeHelper.getProvisionerUuidFileName] && strlen(provisioner_uuid) == 32) {
        WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] active provisioner_uuid: %s\n", provisioner_uuid);
        return 0;
    }

    // create the prov_uuid.bin to stote the UUID string value or read the stored UUID string if existing.
    NSFileHandle *handle;
    NSData *data;
    NSString *filePath = [MeshNativeHelper getProvisionerUuidFileName];
    if ([fileManager fileExistsAtPath:filePath]) {
        handle = [NSFileHandle fileHandleForReadingAtPath:filePath];
        if (handle == nil) {
            WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] error: unable to open file \"%s\" for reading\n", filePath.UTF8String);
            return -3;
        }

        // read back the stored provisoner uuid string from prov_uuid.bin file.
        data = [handle readDataOfLength:32];
        [data getBytes:provisioner_uuid length:(NSUInteger)32];
        provisioner_uuid[32] = '\0';  // always set the terminate character for the provisioner uuid string.
        WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] read provisioner_uuid: %s from %s\n", provisioner_uuid, MeshNativeHelper.getProvisionerUuidFileName.UTF8String);
    } else {
        bRet = [fileManager createFileAtPath:filePath contents:nil attributes:nil];
        handle = [NSFileHandle fileHandleForWritingAtPath:filePath];
        if (!bRet || handle == nil) {
            if (handle) {
                [handle closeFile];
            }
            WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] error: unable to create file \"%s\" for writing\n", filePath.UTF8String);
            return -4;
        }

        // Create a new UUID string for the provisoner and stored to the prov_uuid.bin file.
        // Based on UUID with RFC 4122 version 4, the format is XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX. It's 36 characters.
        // but the UUID format required in the provision_uuid should not including the '-' character,
        // so the UUID string stored in provision_uuid should be 32 characters, it must be conveted here.
        int j = 0;
        char *rfcuuid = NULL;
        if (provisionerUuid == nil) {
            NSUUID *newUuid = MeshNativeHelper.generateRfcUuid;
            rfcuuid = (char *)newUuid.UUIDString.UTF8String;
            [nativeCallbackDelegate updateProvisionerUuid:newUuid];
        } else {
            rfcuuid = (char *)provisionerUuid.UUIDString.UTF8String;
        }
        for (int i = 0; i < strlen(rfcuuid); i++) {
            if (rfcuuid[i] == '-') {
                continue;
            }
            provisioner_uuid[j++] = rfcuuid[i];
        }
        provisioner_uuid[j] = '\0';
        data = [NSData dataWithBytes:provisioner_uuid length:strlen(provisioner_uuid)];
        [handle writeData:data];    // write 32 bytes.
        WICED_BT_TRACE("[MeshNativeHelper updateProvisionerUuid] create provisioner_uuid: %s, and stored to %s\n", provisioner_uuid, MeshNativeHelper.getProvisionerUuidFileName.UTF8String);
    }
    [handle closeFile];
    return 0;
}

/*
 * MeshNativeHelper class functions.
 */

-(void) registerNativeCallback: (id)delegate
{
    WICED_BT_TRACE("[MeshNativeHelper registerNativeCallback]\n");
    nativeCallbackDelegate = delegate;
}

+(int) meshClientNetworkExists:(NSString *) meshName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkExists] meshName: %s\n", meshName.UTF8String);
    return mesh_client_network_exists((char *)[meshName UTF8String]);
}

+(int) meshClientNetworkCreate:(NSString *)provisionerName meshName:(NSString *)meshName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkCreate] provisionerName: %s, provisioner_uuid: %s, meshName: %s\n", provisionerName.UTF8String, provisioner_uuid, meshName.UTF8String);
    int ret;
    EnterCriticalSection();
    [MeshNativeHelper updateProvisionerUuid:nil];
    ret = mesh_client_network_create(provisionerName.UTF8String, provisioner_uuid, (char *)meshName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

void mesh_client_network_opened(uint8_t status) {
    WICED_BT_TRACE("[MeshNativeHelper mesh_client_network_opened] status: %u\n", status);
    [nativeCallbackDelegate meshClientNetworkOpenCb:status];
}
+(int) meshClientNetworkOpen:(NSString *)provisionerName meshName:(NSString *)meshName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkOpen] provisionerName: %s, meshName: %s\n", provisionerName.UTF8String, meshName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_network_open(provisionerName.UTF8String, provisioner_uuid, (char *)meshName.UTF8String, mesh_client_network_opened);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientNetworkDelete:(NSString*)provisionerName meshName:(NSString *)meshName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkDelete] provisionerName: %s, meshName: %s\n", provisionerName.UTF8String, meshName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_network_delete(provisionerName.UTF8String, provisioner_uuid, (char *)meshName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(void) meshClientNetworkClose
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkClose]\n");
    EnterCriticalSection();
    mesh_client_network_close();
    LeaveCriticalSection();
}

+(NSString *) meshClientNetworkExport:(NSString *)meshName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkExport] meshName=%s\n", meshName.UTF8String);
    char *jsonString = NULL;
    EnterCriticalSection();
    jsonString = mesh_client_network_export((char *)meshName.UTF8String);
    LeaveCriticalSection();
    if (jsonString == NULL) {
        return nil;
    }
    return [[NSString alloc] initWithUTF8String:jsonString];
}

+(NSString *) meshClientNetworkImport:(NSString *)provisionerName jsonString:(NSString *)jsonString
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientNetworkImport] provisionerName: %s, provisioner_uuid: %s, jsonString: %s\n", provisionerName.UTF8String, provisioner_uuid, jsonString.UTF8String);
    char *networkName = NULL;
    EnterCriticalSection();
    [MeshNativeHelper updateProvisionerUuid:nil];
    networkName = mesh_client_network_import(provisionerName.UTF8String, provisioner_uuid, (char *)jsonString.UTF8String, mesh_client_network_opened);
    LeaveCriticalSection();

    if (networkName == NULL) {
        return nil;
    }
    return [[NSString alloc] initWithUTF8String:networkName];
}


+(int) meshClientGroupCreate:(NSString *)groupName parentGroupName:(NSString *)parentGroupName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGroupCreate] groupName: %s, parentGroupName: %s\n", groupName.UTF8String, parentGroupName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_group_create((char *)groupName.UTF8String, (char *)parentGroupName.UTF8String);
    LeaveCriticalSection();
    return ret;
}
+(int) meshClientGroupDelete:(NSString *)groupName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGroupDelete] groupName: %s\n", groupName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_group_delete((char *)groupName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

/*
 * This help function will convert the C strings from the input buffer to a NSArray<NSString *> array data,
 * and free C String if required.
 */
NSArray<NSString *> * meshCStringToOCStringArray(const char *cstrings, BOOL freeCString)
{
    NSMutableArray<NSString *> *stringArray = [[NSMutableArray<NSString *> alloc] init];
    char *p_str = (char *)cstrings;

    if (p_str == NULL || *p_str == '\0') {
        return NULL;
    }

    for (int i = 0; p_str != NULL && *p_str != '\0'; p_str += (strlen(p_str) + 1), i++) {
        stringArray[i] = [NSString stringWithUTF8String:p_str];
    }

    if (freeCString) {
        free((void *)cstrings);
    }
    return [NSArray<NSString *> arrayWithArray:stringArray];
}

+(NSArray<NSString *> *) meshClientGetAllNetworks
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetAllNetworks]\n");
    char *networks = mesh_client_get_all_networks();
    return meshCStringToOCStringArray(networks, TRUE);
}

+(NSArray<NSString *> *) meshClientGetAllGroups:(NSString *)inGroup
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetAllGroups] inGroup: %s\n", inGroup.UTF8String);
    char *groups = NULL;
    EnterCriticalSection();
    groups = mesh_client_get_all_groups((char *)inGroup.UTF8String);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(groups, TRUE);
}

+(Boolean) meshClientIsSubGroupName:(NSString *)componentName groupName:(NSString *)groupName
{
    Boolean isGroup = false;
    char *p;
    EnterCriticalSection();
    char *groups = mesh_client_get_all_groups((groupName == nil) ? NULL : (char *)groupName.UTF8String);
    LeaveCriticalSection();
    for (p = groups; (p != NULL) && (*p != 0); p += (strlen(p) + 1)) {
        if (strcmp(componentName.UTF8String, p) == 0) {
            isGroup = true;
            break;
        }
    }
    if (!isGroup) {
        for (p = groups; (p != NULL) && (*p != 0); p += (strlen(p) + 1)) {
            if ([MeshNativeHelper meshClientIsSubGroupName:componentName groupName:[NSString stringWithUTF8String:p]]) {
                isGroup = true;
                break;
            }
        }
    }
    free((void *)groups);
    return isGroup;
}
+(Boolean) meshClientIsGroup:(NSString *)componentName
{
    Boolean isGroup = [MeshNativeHelper meshClientIsSubGroupName:componentName groupName:nil];
    WICED_BT_TRACE("[MeshNativeHelper meshClientIsGroup] %s, %s\n", componentName.UTF8String, isGroup ? "true" : "false");
    return isGroup;
}

+(NSArray<NSString *> *) meshClientGetAllProvisioners
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetAllProvisioners]\n");
    char *provisioners = NULL;
    EnterCriticalSection();
    provisioners = mesh_client_get_all_provisioners();
    LeaveCriticalSection();
    return meshCStringToOCStringArray(provisioners, TRUE);
}

+(NSArray<NSString *> *) meshClientGetDeviceComponents:(NSUUID *)uuid
{
    char *components = NULL;
    uint8_t p_uuid[16];
    [uuid getUUIDBytes:p_uuid];
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetDeviceComponents] device uuid: %s\n", uuid.UUIDString.UTF8String);
    EnterCriticalSection();
    components = mesh_client_get_device_components(p_uuid);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(components, TRUE);
}

+(NSArray<NSString *> *) meshClientGetGroupComponents:(NSString *)groupName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetGroupComponents] groupName: %s\n", groupName.UTF8String);
    char *componetNames = NULL;
    EnterCriticalSection();
    componetNames = mesh_client_get_group_components((char *)groupName.UTF8String);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(componetNames, TRUE);
}

+(NSArray<NSString *> *) meshClientGetTargetMethods:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetTargetMethods] componentName: %s\n", componentName.UTF8String);
    char *targetMethods = NULL;
    EnterCriticalSection();
    targetMethods = mesh_client_get_target_methods(componentName.UTF8String);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(targetMethods, TRUE);
}

+(NSArray<NSString *> *) meshClientGetControlMethods:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetControlMethods] componentName: %s\n", componentName.UTF8String);
    char *controlMethods = NULL;
    EnterCriticalSection();
    controlMethods = mesh_client_get_control_methods(componentName.UTF8String);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(controlMethods, TRUE);
}

+(uint8_t) meshClientGetComponentType:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetComponentType] componentName: %s\n", componentName.UTF8String);
    uint8_t type;
    EnterCriticalSection();
    type = mesh_client_get_component_type((char *)componentName.UTF8String);
    LeaveCriticalSection();
    return type;
}

void meshClientComponentInfoStatusCallback(uint8_t status, char *component_name, char *component_info)
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] status:0x%x\n", status);
    NSString *componentName = nil;
    NSString *componentInfo = nil;
    if (component_name == NULL) {
        WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] error, invalid parameters, component_name: is NULL\n");
        return;
    }
    componentName = [NSString stringWithUTF8String:(const char *)component_name];
    if (component_info != NULL) {
        if (strstr(component_info, "Not Available") == NULL) {
            uint8_t firmware_id[8];
            memcpy(firmware_id, &component_info[34], 8);
            component_info[34] = '\0';
            WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] firmware ID: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                   firmware_id[0], firmware_id[1], firmware_id[2], firmware_id[3],
                   firmware_id[4], firmware_id[5], firmware_id[6], firmware_id[7]);
            uint16_t pid = (uint16_t)(((uint16_t)(firmware_id[0]) << 8) | (uint16_t)(firmware_id[1]));
            uint16_t hwid = (uint16_t)(((uint16_t)(firmware_id[2]) << 8) | (uint16_t)(firmware_id[3]));
            uint8_t fwVerMaj = (uint8_t)firmware_id[4];
            uint8_t fwVerMin = (uint8_t)firmware_id[5];
            uint16_t fwVerRev = (uint16_t)(((uint16_t)(firmware_id[6]) << 8) | (uint16_t)(firmware_id[7]));
            WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] Product ID:0x%04x (%u), HW Version ID:0x%04x (%u), Firmware Version, %u.%u.%u\n",
                   pid, pid, hwid, hwid, fwVerMaj, fwVerMin, fwVerRev);
            char info[128] = { 0 };
            sprintf(info, "%s%d.%d.%d", component_info, fwVerMaj, fwVerMin, fwVerRev);
            componentInfo = [NSString stringWithUTF8String:(const char *)info];
        } else {
            componentInfo = [NSString stringWithUTF8String:(const char *)component_info];
        }

        WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] componentInfo:%s\n", componentInfo.UTF8String);
    } else {
        WICED_BT_TRACE("[MeshNativeHelper meshClientComponentInfoStatusCallback] component_info string is NULL\n");
    }
    [nativeCallbackDelegate meshClientComponentInfoStatusCb:status componentName:componentName componentInfo:componentInfo];
}

+(uint8_t) meshClientGetComponentInfo:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetComponentInfo] componentName:%s\n", componentName.UTF8String);
    uint8_t ret;
    EnterCriticalSection();
    ret = mesh_client_get_component_info((char *)componentName.UTF8String, meshClientComponentInfoStatusCallback);
    LeaveCriticalSection();
    return ret;
}

/*
 * When the controlMethod is nil or empty, the library will register to receive messages sent to all type of messages.
 * When the groupName is nil or empty, the library will register to receive messages sent to all the groups.
 */
+(int) meshClientListenForAppGroupBroadcasts:(NSString *)controlMethod groupName:(NSString *)groupName startListen:(BOOL)startListen
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientListenForAppGroupBroadcasts] controlMethod:%s, groupName:%s, startListen:%d\n", controlMethod.UTF8String, groupName.UTF8String, (int)startListen);
    char *listonControlMethod = (controlMethod != nil && controlMethod.length > 0) ? (char *)controlMethod.UTF8String : NULL;
    char *listonGroupName = (groupName != nil && groupName.length > 0) ? (char *)groupName.UTF8String : NULL;
    int ret;

    EnterCriticalSection();
    ret = mesh_client_listen_for_app_group_broadcasts(listonControlMethod, listonGroupName, (wiced_bool_t)startListen);
    LeaveCriticalSection();
    return ret;
}

+(NSString *) meshClientGetPublicationTarget:(NSString *)componentName isClient:(BOOL)isClient method:(NSString *)method
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetPublicationTarget] componentName:%s, isClient:%d, method:%s\n", componentName.UTF8String, (int)isClient, method.UTF8String);
    const char *targetName;
    EnterCriticalSection();
    targetName = mesh_client_get_publication_target(componentName.UTF8String, (uint8_t)isClient, method.UTF8String);
    LeaveCriticalSection();
    return (targetName == NULL) ? nil : [NSString stringWithUTF8String:targetName];
}

/*
 * Return 0 on failed to to get publication period or encountered any error.
 * Otherwise, return the publish period value on success.
 */
+(int) meshClientGetPublicationPeriod:(NSString *)componentName isClient:(BOOL)isClient method:(NSString *)method
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetPublicationPeriod] componentName:%s, isClient:%d, method:%s\n", componentName.UTF8String, (int)isClient, method.UTF8String);
    int publishPeriod;
    EnterCriticalSection();
    publishPeriod = mesh_client_get_publication_period((char *)componentName.UTF8String, (uint8_t)isClient, method.UTF8String);
    LeaveCriticalSection();
    return publishPeriod;
}

+(int) meshClientRename:(NSString *)oldName newName:(NSString *)newName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientRename] oldName:%s, newName:%s\n", oldName.UTF8String, newName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_rename((char *)oldName.UTF8String, (char *)newName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientMoveComponentToGroup:(NSString *)componentName from:(NSString *)fromGroupName to:(NSString *)toGroupName
{
    WICED_BT_TRACE("[MeshNativehelper meshClientMoveComponentToGroup] componentName:%s, fromGroupName:%s, toGroupName:%s\n",
          componentName.UTF8String, fromGroupName.UTF8String, toGroupName.UTF8String);
    int ret;
    NSString *networkName = MeshNativeHelper.meshClientGetNetworkName;
    if (networkName == nil) {
        return MESH_CLIENT_ERR_NETWORK_CLOSED;
    }

    // might need to use default pub parameters.
    FILE *fp = fopen(MESH_DEVICE_CONFIG_PARAMS_FILE_NAME, "wb");
    if (fp) {
        fwrite(&mesh_device_config_params, 1, sizeof(mesh_device_config_params), fp);
        fclose(fp);
    }
    mesh_client_set_publication_config(mesh_device_config_params.publish_credential_flag,
                                       mesh_device_config_params.publish_retransmit_count,
                                       mesh_device_config_params.publish_retransmit_interval,
                                       mesh_device_config_params.publish_ttl);

    EnterCriticalSection();
    if (((fromGroupName.length == 0) || [networkName isEqualToString:fromGroupName]) && ((toGroupName.length != 0) && ![networkName isEqualToString:toGroupName])) {
        ret = mesh_client_add_component_to_group(componentName.UTF8String, toGroupName.UTF8String);
    } else if (((fromGroupName.length != 0) || ![networkName isEqualToString:fromGroupName]) && ((toGroupName.length == 0) || [networkName isEqualToString:toGroupName])) {
        ret = mesh_client_remove_component_from_group(componentName.UTF8String, fromGroupName.UTF8String);
    } else {
        ret = mesh_client_move_component_to_group(componentName.UTF8String, fromGroupName.UTF8String, toGroupName.UTF8String);
    }
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientConfigurePublication:(NSString *)componentName isClient:(uint8_t)isClient method:(NSString *)method targetName:(NSString *)targetName publishPeriod:(int)publishPeriod
{
    WICED_BT_TRACE("[MeshNativehelper meshClientConfigurePublication] componentName:%s, isClient:%d, method:%s, targetName:%s publishPeriod:%d\n",
          componentName.UTF8String, isClient, method.UTF8String, targetName.UTF8String, publishPeriod);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_configure_publication(componentName.UTF8String, isClient, method.UTF8String, targetName.UTF8String, publishPeriod);
    LeaveCriticalSection();
    return ret;
}

+(uint8_t) meshClientProvision:(NSString *)deviceName groupName:(NSString *)groupName uuid:(NSUUID *)uuid identifyDuration:(uint8_t)identifyDuration
{
    WICED_BT_TRACE("[MeshNativehelper meshClientProvision] deviceName:%s, groupName:%s, identifyDuration:%d\n", deviceName.UTF8String, groupName.UTF8String, identifyDuration);
    uint8_t ret;
    uint8_t p_uuid[16];
    [uuid getUUIDBytes:p_uuid];
    EnterCriticalSection();
    ret = mesh_client_provision(deviceName.UTF8String, groupName.UTF8String, p_uuid, identifyDuration);
    LeaveCriticalSection();
    return ret;
}

+(uint8_t) meshClientConnectNetwork:(uint8_t)useGattProxy scanDuration:(uint8_t)scanDuration
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientConnectNetwork] useGattProxy:%d, scanDuration:%d\n", useGattProxy, scanDuration);
    uint8_t ret;
    EnterCriticalSection();
    ret = mesh_client_connect_network(useGattProxy, scanDuration);
    LeaveCriticalSection();
    return ret;
}

+(uint8_t) meshClientDisconnectNetwork:(uint8_t)useGattProxy
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientDisconnectNetwork] useGattProxy:%d\n", useGattProxy);
    uint8_t ret;
    EnterCriticalSection();
    ret = mesh_client_disconnect_network();
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientOnOffGet:(NSString *)deviceName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientOnOffGet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_on_off_get(deviceName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientOnOffSet:(NSString *)deviceName onoff:(uint8_t)onoff reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:deviceName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativehelper meshClientOnOffSet] deviceName:%s, onoff:%d, reliable:%d, applied_reliable:%d, transitionTime:%d, delay:%d\n",
          deviceName.UTF8String, onoff, reliable, applied_reliable, transitionTime, delay);
    EnterCriticalSection();
    ret = mesh_client_on_off_set(deviceName.UTF8String, onoff, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientLevelGet:(NSString *)deviceName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientLevelGet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_level_get(deviceName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientLevelSet:(NSString *)deviceName level:(int16_t)level reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:deviceName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativehelper meshClientLevelSet] deviceName:%s, level:%d, reliable:%d, applied_reliable:%d, transitionTime:%d, delay:%d\n",
          deviceName.UTF8String, level, reliable, applied_reliable, transitionTime, delay);
    EnterCriticalSection();
    ret = mesh_client_level_set(deviceName.UTF8String, (int16_t)level, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientHslGet:(NSString *)deviceName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientHslGet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_hsl_get(deviceName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientHslSet:(NSString *)deviceName lightness:(uint16_t)lightness hue:(uint16_t)hue saturation:(uint16_t)saturation reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:deviceName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativehelper meshClientHslSet] deviceName:%s, lightness:%d, hue:%d, saturation:%d, reliable:%d, applied_reliable:%d, transitionTime:%d, delay:%d\n",
          deviceName.UTF8String, lightness, hue, saturation, reliable, applied_reliable, transitionTime, delay);
    EnterCriticalSection();
    ret = mesh_client_hsl_set(deviceName.UTF8String, lightness, hue, saturation, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

+(void) meshClientInit
{
    WICED_BT_TRACE("[MeshNativehelper meshClientInit]\n");
    @synchronized (MeshNativeHelper.getSharedInstance) {
        srand((unsigned int)time(NULL));    // Set the seed value to avoid same pseudo-random intergers are generated.
        if (!initTimer()) {                 // The the global shared recurive mutex lock 'cs' befer using it.
            WICED_BT_TRACE("[MeshNativehelper meshClientInit] error: failed to initiaze timer and shared recurive mutex lock. Stopped\n");
            return;
        }

        FILE *fp = fopen(MESH_DEVICE_CONFIG_PARAMS_FILE_NAME, "rb");
        if (fp) {
            fread(&mesh_device_config_params, 1, sizeof(mesh_device_config_params), fp);
            fclose(fp);
        }

        mesh_client_init(&mesh_client_init_callback);
    }
}

+(int) meshClientSetDeviceConfig:(NSString *)deviceName
                     isGattProxy:(int)isGattProxy
                        isFriend:(int)isFriend
                         isRelay:(int)isRelay
                          beacon:(int)beacon
                  relayXmitCount:(int)relayXmitCount
               relayXmitInterval:(int)relayXmitInterval
                      defaultTtl:(int)defaultTtl
                    netXmitCount:(int)netXmitCount
                 netXmitInterval:(int)netXmitInterval
{
    WICED_BT_TRACE("[MeshNativehelper meshClientSetDeviceConfig] deviceName:%s, isGattProxy:%d, isFriend:%d, isRelay:%d, beacon:%d, relayXmitCount:%d, relayXmitInterval:%d, defaultTtl:%d, netXmitCount:%d, netXmitInterval%d\n",
          deviceName.UTF8String, isGattProxy, isFriend, isRelay, beacon, relayXmitCount, relayXmitInterval, defaultTtl, netXmitCount, netXmitInterval);
    int ret;
    char *device_name = NULL;
    if (deviceName != NULL && deviceName.length > 0) {
        device_name = (char *)deviceName.UTF8String;
    }

    mesh_device_config_params.is_gatt_proxy = isGattProxy;
    mesh_device_config_params.is_friend = isFriend;
    mesh_device_config_params.is_relay = isRelay;
    mesh_device_config_params.send_net_beacon = beacon;
    mesh_device_config_params.relay_xmit_count = relayXmitCount;
    mesh_device_config_params.relay_xmit_interval = relayXmitInterval;
    mesh_device_config_params.default_ttl = defaultTtl;
    mesh_device_config_params.net_xmit_count = netXmitCount;
    mesh_device_config_params.net_xmit_interval = netXmitInterval;
    FILE *fp = fopen(MESH_DEVICE_CONFIG_PARAMS_FILE_NAME, "wb");
    if (fp) {
        fwrite(&mesh_device_config_params, 1, sizeof(mesh_device_config_params), fp);
        fclose(fp);
    }

    EnterCriticalSection();
    ret = mesh_client_set_device_config(device_name,
                                        isGattProxy,
                                        isFriend,
                                        isRelay,
                                        beacon,
                                        relayXmitCount,
                                        relayXmitInterval,
                                        defaultTtl,
                                        netXmitCount,
                                        netXmitInterval);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSetPublicationConfig:(int)publishCredentialFlag
               publishRetransmitCount:(int)publishRetransmitCount
            publishRetransmitInterval:(int)publishRetransmitInterval
                           publishTtl:(int)publishTtl
{
    WICED_BT_TRACE("[MeshNativehelper meshClientSetPublicationConfig] publishCredentialFlag:%d, publishRetransmitCount:%d, publishRetransmitInterval:%d, publishTtl:%d\n",
          publishCredentialFlag, publishRetransmitCount, publishRetransmitInterval, publishTtl);
    int ret;

    mesh_device_config_params.publish_credential_flag = publishCredentialFlag;
    mesh_device_config_params.publish_ttl = publishTtl;
    mesh_device_config_params.publish_retransmit_count = publishRetransmitCount;
    mesh_device_config_params.publish_retransmit_interval = publishRetransmitInterval;
    FILE *fp = fopen(MESH_DEVICE_CONFIG_PARAMS_FILE_NAME, "wb");
    if (fp) {
        fwrite(&mesh_device_config_params, 1, sizeof(mesh_device_config_params), fp);
        fclose(fp);
    }

    EnterCriticalSection();
    ret = mesh_client_set_publication_config(publishCredentialFlag,
                                             publishRetransmitCount,
                                             publishRetransmitInterval,
                                             publishTtl);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientResetDevice:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientResetDevice] componentName:%s\n", componentName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_reset_device((char *)componentName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientVendorDataSet:(NSString *)deviceName companyId:(uint16_t)companyId modelId:(uint16_t)modelId opcode:(uint8_t)opcode disable_ntwk_retransmit:(uint8_t)disable_ntwk_retransmit data:(NSData *)data
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientVendorDataSet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_vendor_data_set(deviceName.UTF8String, companyId, modelId, opcode, disable_ntwk_retransmit, (uint8_t *)data.bytes, data.length);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientIdentify:(NSString *)name duration:(uint8_t)duration
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientIdentify] name:%s, duration:%d\n", name.UTF8String, duration);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_identify(name.UTF8String, duration);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientLightnessGet:(NSString *)deviceName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientLightnessGet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_lightness_get(deviceName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientLightnessSet:(NSString *)deviceName lightness:(uint16_t)lightness reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:deviceName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativehelper meshClientLightnessSet] deviceName:%s, lightness:%d, reliable:%d, applied_reliable:%d, transitionTime:%d, delay:%d\n",
          deviceName.UTF8String, lightness, reliable, applied_reliable, transitionTime, delay);
    EnterCriticalSection();
    ret = mesh_client_lightness_set(deviceName.UTF8String, lightness, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientCtlGet:(NSString *)deviceName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientCtlGet] deviceName:%s\n", deviceName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_ctl_get(deviceName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientCtlSet:(NSString *)deviceName lightness:(uint16_t)lightness temperature:(uint16_t)temperature deltaUv:(uint16_t)deltaUv
               reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:deviceName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativeHelper meshClientCtlSet] deviceName: %s, lightness: %d, temperature: %d, deltaUv: %d, reliable: %d, applied_reliable:%d, transitionTime: %d, delay: %d\n",
          deviceName.UTF8String, lightness, temperature, deltaUv, reliable, applied_reliable, transitionTime, delay);
    EnterCriticalSection();
    ret = mesh_client_ctl_set(deviceName.UTF8String, lightness, temperature, deltaUv, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

//MESH CLIENT GATT APIS
+(void) meshClientScanUnprovisioned:(int)start uuid:(NSData *)uuid
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientScanUnprovisioned] start:%d\n", start);
    EnterCriticalSection();
    mesh_client_scan_unprovisioned(start, (uuid != nil && uuid.length == MESH_DEVICE_UUID_LEN) ? (uint8_t *)uuid.bytes : NULL);
    LeaveCriticalSection();
}

+(Boolean) meshClientIsConnectingProvisioning
{
    Boolean is_connecting_provisioning;
    EnterCriticalSection();
    is_connecting_provisioning = mesh_client_is_connecting_provisioning();
    LeaveCriticalSection();
    WICED_BT_TRACE("[MeshNativeHelper meshClientIsConnectingProvisioning] is_connecting_provisioning=%d\n", is_connecting_provisioning);
    return is_connecting_provisioning;
}

+(void) meshClientConnectionStateChanged:(uint16_t)connId mtu:(uint16_t)mtu
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientConnectionStateChanged] connId:0x%04x, mtu:%d\n", connId, mtu);
    mesh_client_connection_state_changed(connId, mtu);
}

+(void) meshClientAdvertReport:(NSData *)bdaddr addrType:(uint8_t)addrType rssi:(int8_t)rssi advData:(NSData *) advData
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientAdvertReport] advData.length:%lu, rssi:%d\n", (unsigned long)advData.length, rssi);
    if (bdaddr.length == 6 && advData.length > 0) {
        mesh_client_advert_report((uint8_t *)bdaddr.bytes, addrType, rssi, (uint8_t *)advData.bytes);
    } else {
        WICED_BT_TRACE("[MeshNativeHelper meshClientAdvertReport] error: invalid bdaddr or advdata, bdaddr.length=%lu, advData.length=%lu\n",
              (unsigned long)bdaddr.length, (unsigned long)advData.length);
    }
}

+(uint8_t) meshConnectComponent:(NSString *)componentName useProxy:(uint8_t)useProxy scanDuration:(uint8_t)scanDuration
{
    uint8_t ret;
    EnterCriticalSection();
    ret = mesh_client_connect_component((char *)componentName.UTF8String, useProxy, scanDuration);
    WICED_BT_TRACE("[MeshNativehelper meshConnectComponent] componentName: %s, useProxy: %d, scanDuration: %d, error: %d\n", componentName.UTF8String, useProxy, scanDuration, ret);
    if (ret != MESH_CLIENT_SUCCESS) {
        meshClientNodeConnectionState(MESH_CLIENT_NODE_WARNING_UNREACHABLE, (char *)componentName.UTF8String);
    }
    LeaveCriticalSection();
    return ret;
}

+(void) sendRxProxyPktToCore:(NSData *)data
{
    WICED_BT_TRACE("[MeshNativeHelper sendRxProxyPktToCore] data.length: %lu\n", (unsigned long)data.length);
    EnterCriticalSection();
    mesh_client_proxy_data((uint8_t *)data.bytes, data.length);
    LeaveCriticalSection();
}

+(void) sendRxProvisPktToCore:(NSData *)data
{
    WICED_BT_TRACE("[MeshNativeHelper sendRxProvisPktToCore] data.length: %lu\n", (unsigned long)data.length);
    EnterCriticalSection();
    mesh_client_provisioning_data(WICED_TRUE, (uint8_t *)data.bytes, data.length);
    LeaveCriticalSection();
}

+(Boolean) isMeshProvisioningServiceAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData
{
    CBUUID *provUuid = [CBUUID UUIDWithString:@"1827"];

    NSNumber *conntable = advertisementData[CBAdvertisementDataIsConnectable];
    if (conntable == nil || [conntable isEqual: [NSNumber numberWithUnsignedInteger:0]]) {
        return false;
    }

    NSArray *srvUuids = advertisementData[CBAdvertisementDataServiceUUIDsKey];
    if (srvUuids == nil || srvUuids.count == 0) {
        return false;
    }

    for (CBUUID *uuid in srvUuids) {
        if ([uuid isEqual: provUuid]) {
            NSDictionary *srvData = advertisementData[CBAdvertisementDataServiceDataKey];
            if (srvData != nil && srvData.count > 0) {
                NSArray *allKeys = [srvData allKeys];
                for (CBUUID *key in allKeys) {
                    if (![key isEqual:provUuid]) {
                        continue;
                    }
                    NSData *data = srvData[key];
                    if (data != nil && data.length > 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

+(Boolean) isMeshProxyServiceAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData
{
    CBUUID *proxyUuid = [CBUUID UUIDWithString:@"1828"];

    NSNumber *conntable = advertisementData[CBAdvertisementDataIsConnectable];
    if (conntable == nil || [conntable isEqual: [NSNumber numberWithUnsignedInteger:0]]) {
        return false;
    }

    NSArray *srvUuids = advertisementData[CBAdvertisementDataServiceUUIDsKey];
    if (srvUuids == nil || srvUuids.count == 0) {
        return false;
    }

    for (CBUUID *uuid in srvUuids) {
        if ([uuid isEqual: proxyUuid]) {
            NSDictionary *srvData = advertisementData[CBAdvertisementDataServiceDataKey];
            if (srvData != nil && srvData.count > 0) {
                NSArray *allKeys = [srvData allKeys];
                for (CBUUID *key in allKeys) {
                    if (![key isEqual:proxyUuid]) {
                        continue;
                    }
                    NSData *data = srvData[key];
                    if (data != nil && data.length > 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

+(Boolean) isMeshAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData
{
    CBUUID *provUuid = [CBUUID UUIDWithString:@"1827"];
    CBUUID *proxyUuid = [CBUUID UUIDWithString:@"1828"];

    NSNumber *conntable = advertisementData[CBAdvertisementDataIsConnectable];
    if (conntable == nil || [conntable isEqual: [NSNumber numberWithUnsignedInteger:0]]) {
        return false;
    }

    NSArray *srvUuids = advertisementData[CBAdvertisementDataServiceUUIDsKey];
    if (srvUuids == nil || srvUuids.count == 0) {
        return false;
    }

    for (CBUUID *uuid in srvUuids) {
        if ([uuid isEqual: provUuid] || [uuid isEqual: proxyUuid]) {
            NSDictionary *srvData = advertisementData[CBAdvertisementDataServiceDataKey];
            if (srvData != nil && srvData.count > 0) {
                NSArray *allKeys = [srvData allKeys];
                for (CBUUID *key in allKeys) {
                    if (![key isEqual:provUuid] && ![key isEqual:proxyUuid]) {
                        continue;
                    }
                    NSData *data = srvData[key];
                    if (data != nil && data.length > 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static NSMutableDictionary *gMeshBdAddrDict = nil;
-(void) meshBdAddrDictInit
{
    if (gMeshBdAddrDict == nil) {
        EnterCriticalSection();
        if (gMeshBdAddrDict == nil) {
            gMeshBdAddrDict = [[NSMutableDictionary alloc] init];
        }
        LeaveCriticalSection();
    }
}

- (void)destoryMeshClient
{
    [self deleteAllFiles];
    onceToken = 0;
    zoneOnceToken = 0;
    [gMeshBdAddrDict removeAllObjects];
    gMeshBdAddrDict = nil;
    for (NSString *key in [gMeshTimersDict allKeys]) {
        NSArray *timerInfo = [gMeshTimersDict valueForKey:key];
        if (timerInfo != nil && [timerInfo count] == 3) {
            NSTimer *timer = timerInfo[2];
            if (timer != nil) {
                [timer invalidate];
            }
        }
    }
    [gMeshTimersDict removeAllObjects];
    gMeshTimersDict = nil;
    strcpy(provisioner_uuid, "");
    _instance = nil;
}

+(void) meshBdAddrDictAppend:(NSData *)bdAddr peripheral:(CBPeripheral *)peripheral
{
    if (bdAddr == nil || bdAddr.length != BD_ADDR_LEN || peripheral == nil) {
        return;
    }
    [gMeshBdAddrDict setObject:peripheral forKey:bdAddr.description];
}

+(CBPeripheral *) meshBdAddrDictGetCBPeripheral:(NSData *)bdAddr
{
    if (bdAddr == nil || bdAddr.length != BD_ADDR_LEN) {
        return nil;
    }
    return [gMeshBdAddrDict valueForKey:bdAddr.description];
}

+(void) meshBdAddrDictDelete:(NSData *)bdAddr
{
    if (bdAddr == nil || bdAddr.length != BD_ADDR_LEN) {
        return;
    }
    [gMeshBdAddrDict removeObjectForKey:bdAddr.description];
}

+(void) meshBdAddrDictDeleteByCBPeripheral:(CBPeripheral *)peripheral
{
    if (peripheral == nil) {
        return;
    }
    for (NSData *bdAddr in [gMeshBdAddrDict allKeys]) {
        CBPeripheral *cachedPeripheral = [gMeshBdAddrDict valueForKey:bdAddr.description];
        if (peripheral == cachedPeripheral) {
            [gMeshBdAddrDict removeObjectForKey:bdAddr.description];
            break;
        }
    }
}

+(void) meshBdAddrDictClear
{
    [gMeshBdAddrDict removeAllObjects];
}

+(NSData *) MD5:(NSData *)data
{
    unsigned char md5Data[CC_MD5_DIGEST_LENGTH];
    memset(md5Data, 0, CC_MD5_DIGEST_LENGTH);
    CC_MD5(data.bytes, (unsigned int)data.length, md5Data);
    return [[NSData alloc] initWithBytes:md5Data length:CC_MD5_DIGEST_LENGTH];
}

+(NSData *)peripheralIdentifyToBdAddr:(CBPeripheral *)peripheral
{
    const char *uuid = peripheral.identifier.UUIDString.UTF8String;
    unsigned char md5Data[CC_MD5_DIGEST_LENGTH];
    CC_MD5(uuid, (unsigned int)strlen(uuid), md5Data);
    return [[NSData alloc] initWithBytes:md5Data length:BD_ADDR_LEN];
}

+(NSData *) getMeshPeripheralMappedBdAddr:(CBPeripheral *)peripheral
{
    NSData * bdAddr = [MeshNativeHelper peripheralIdentifyToBdAddr:peripheral];
    [MeshNativeHelper meshBdAddrDictAppend:bdAddr peripheral:peripheral];
    return bdAddr;
}

#define MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE    62          /* max is 31 advertisement data combined with 31 scan response data */
+(NSData *) getConvertedRawMeshAdvertisementData:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData rssi:(NSNumber *)rssi
{
    BOOL isConntable = false;
    BOOL isMeshServiceFound = false;
    CBUUID *provUuid = [CBUUID UUIDWithString:@"1827"];
    CBUUID *proxyUuid = [CBUUID UUIDWithString:@"1828"];
    /* assume the advertisementData was combined with max 31 bytes advertisement data and max 31 bytes scan response data. */
    unsigned char rawAdvData[MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE];
    int rawAdvDataSize = 0;
    unsigned char *p;

    NSNumber *conntable = advertisementData[CBAdvertisementDataIsConnectable];
    if (conntable != nil && [conntable isEqual: [NSNumber numberWithUnsignedInteger:1]]) {
        isConntable = true;
        if ((rawAdvDataSize + 1 + 2) <= MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE) {
            rawAdvData[rawAdvDataSize++] = 2;                           // length
            rawAdvData[rawAdvDataSize++] = BTM_BLE_ADVERT_TYPE_FLAG;    // flag type
            rawAdvData[rawAdvDataSize++] = 0x06;                        // Flags value
        }
    }

    NSArray *srvUuids = advertisementData[CBAdvertisementDataServiceUUIDsKey];
    if (srvUuids != nil && srvUuids.count > 0) {
        for (CBUUID *uuid in srvUuids) {
            if ([uuid isEqual: provUuid] || [uuid isEqual: proxyUuid]) {
                isMeshServiceFound = true;
                if ((rawAdvDataSize + uuid.data.length + 2) <= MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE) {
                    rawAdvData[rawAdvDataSize++] = uuid.data.length + 1;                    // length
                    rawAdvData[rawAdvDataSize++] = BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE;      // flag type
                    /* UUID data bytes is in big-endia format, but raw adv data wants in little-endian format. */
                    p = (unsigned char *)uuid.data.bytes;
                    p += (uuid.data.length - 1);
                    for (int i = 0; i < uuid.data.length; i++) {
                        rawAdvData[rawAdvDataSize++] = *p--;                                // little-endian UUID data
                    }
                }
            }
        }
    }

    /* Not mesh device/proxy advertisement data, return nil. */
    if (!isConntable || !isMeshServiceFound) {
        return nil;
    }

    NSDictionary *srvData = advertisementData[CBAdvertisementDataServiceDataKey];
    if (srvData != nil && srvData.count > 0) {
        NSArray *allKeys = [srvData allKeys];
        for (CBUUID *key in allKeys) {
            if (![key isEqual:provUuid] && ![key isEqual:proxyUuid]) {
                continue;
            }

            NSData *data = srvData[key];
            if (data != nil && data.length > 0) {
                if ((rawAdvDataSize + key.data.length + data.length + 2) <= MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE) {
                    rawAdvData[rawAdvDataSize++] = key.data.length + data.length + 1;   // length
                    rawAdvData[rawAdvDataSize++] = BTM_BLE_ADVERT_TYPE_SERVICE_DATA;    // flag type
                    p = (unsigned char *)key.data.bytes;                                // data: service UUID.
                    p += (key.data.length - 1);
                    for (int i = 0; i < key.data.length; i++) {
                        rawAdvData[rawAdvDataSize++] = *p--;
                    }
                    memcpy(&rawAdvData[rawAdvDataSize], data.bytes, data.length);       // data: service UUID data.
                    rawAdvDataSize += data.length;
                }
            }
        }
    }

    /*
     * Add local name of the peripheral if exsiting.
     * When both name existing, use the iOS system processed peripheral.name as the first priority.
     */
    NSString *localName = advertisementData[CBAdvertisementDataLocalNameKey];
    if (localName == nil || strlen(localName.UTF8String) == 0 ||
        (peripheral.name != nil && strlen(peripheral.name.UTF8String))) {
        localName = peripheral.name;
    }
    if (localName != nil && strlen(localName.UTF8String) > 0) {
        unsigned long nameLen = strlen(localName.UTF8String);
        if ((rawAdvDataSize + nameLen + 2) <= MESH_MAX_RAW_ADVERTISEMENT_DATA_SIZE) {
            rawAdvData[rawAdvDataSize++] = nameLen + 1;                            // length
            rawAdvData[rawAdvDataSize++] = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;      // flag type
            memcpy(&rawAdvData[rawAdvDataSize], localName.UTF8String, nameLen);    // Name data
            rawAdvDataSize += nameLen;
        }
    }

    // Add 2 ending bytes, 0 length with 0 flag.
    rawAdvData[rawAdvDataSize++] = 0;
    rawAdvData[rawAdvDataSize++] = 0;

    return [[NSData alloc] initWithBytes:rawAdvData length:rawAdvDataSize];
}

+(void) meshClientSetGattMtu:(int)mtu
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSetGattMtu] mtu: %d\n", mtu);
    EnterCriticalSection();
    wiced_bt_mesh_core_set_gatt_mtu((uint16_t)mtu);
    LeaveCriticalSection();
}
+(Boolean) meshClientIsConnectedToNetwork
{
    Boolean is_proxy_connected;
    EnterCriticalSection();
    is_proxy_connected = mesh_client_is_proxy_connected();
    LeaveCriticalSection();
    WICED_BT_TRACE("[MeshNativeHelper meshClientIsConnectedToNetwork] is_proxy_connected=%d\n", is_proxy_connected);
    return is_proxy_connected;
}

+(int) meshClientAddComponent:(NSString *)componentName toGorup:(NSString *)groupName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientAddComponent] componentName: %s, groupName: %s\n", componentName.UTF8String, groupName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_add_component_to_group(componentName.UTF8String, groupName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

+(NSData *) meshClientOTADataEncrypt:(NSString *)componentName data:(NSData *)data
{
    //WICED_BT_TRACE("[MeshNativeHelper meshClientOTADataEncrypt] componentName: %s, length: %lu\n", componentName.UTF8String, (unsigned long)[data length]);
    /* The output buffer should be at least 17 bytes larger than input buffer */
    uint8_t *pOutBuffer = (uint8_t *)malloc(data.length + 17);
    uint16_t outBufferLen = 0;
    NSData *outData = nil;

    if (pOutBuffer) {
        EnterCriticalSection();
        outBufferLen = mesh_client_ota_data_encrypt(componentName.UTF8String, data.bytes, data.length, pOutBuffer, data.length + 17);
        LeaveCriticalSection();
        if (outBufferLen > 0) {
            outData = [[NSData alloc] initWithBytes:pOutBuffer length:outBufferLen];
        }
    }
    free(pOutBuffer);
    return outData;
}

+(NSData *) meshClientOTADataDecrypt:(NSString *)componentName data:(NSData *)data
{
    //WICED_BT_TRACE("[MeshNativeHelper meshClientOTADataDecrypt] componentName: %s, length: %lu\n", componentName.UTF8String, (unsigned long)[data length]);
    /* The output buffer should be at least 17 bytes larger than input buffer */
    uint8_t *pOutBuffer = (uint8_t *)malloc(data.length + 17);
    uint16_t outBufferLen = 0;
    NSData *outData = nil;

    if (pOutBuffer) {
        EnterCriticalSection();
        outBufferLen = mesh_client_ota_data_decrypt(componentName.UTF8String, data.bytes, data.length, pOutBuffer, data.length + 17);
        LeaveCriticalSection();
        if (outBufferLen > 0) {
            outData = [[NSData alloc] initWithBytes:pOutBuffer length:outBufferLen];
        }
    }
    free(pOutBuffer);
    return outData;
}

+(NSArray<NSString *> *) meshClientGetComponentGroupList:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetComponentGroupList] componentName: %s\n", componentName.UTF8String);
    char *groupList = NULL;
    EnterCriticalSection();
    groupList = mesh_client_get_component_group_list((char *)componentName.UTF8String);
    LeaveCriticalSection();
    return meshCStringToOCStringArray(groupList, TRUE);
}

+(int) meshClientRemoveComponent:(NSString *)componentName from:(NSString *)groupName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientRemoveComponent] componentName:%s, groupName:%s\n", componentName.UTF8String, groupName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_remove_component_from_group(componentName.UTF8String, groupName.UTF8String);
    LeaveCriticalSection();
    return ret;
}

-(NSString *) getJsonFilePath {

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *list = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true);
    NSString *homeDirectory = list[0];
    NSString *fileDirectory = [homeDirectory stringByAppendingPathComponent:@"mesh"];
    WICED_BT_TRACE("[MeshNativeHelper getJsonFilePath] current fileDirectory \"%s\"\n", fileDirectory.UTF8String);

    NSString *fileContent;
    NSArray  *contents = [fileManager contentsOfDirectoryAtPath:fileDirectory error:nil];
    NSString *filePathString;

    for (fileContent in contents){
        if([[fileContent pathExtension]isEqualToString:@"json"]){
            WICED_BT_TRACE("  %s\n",fileContent.UTF8String);
            filePathString = fileContent;
            break;
        }
    }

    NSString *filePath = [fileDirectory stringByAppendingPathComponent:filePathString];
    return filePath;
}

-(void) deleteAllFiles {

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *list = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true);
    NSString *homeDirectory = list[0];
    NSString *fileDirectory = [homeDirectory stringByAppendingPathComponent:@"mesh"];
    WICED_BT_TRACE("[MeshNativeHelper deleteAllFiles] current fileDirectory \"%s\"\n", fileDirectory.UTF8String);

    NSString *fileContent;
    NSArray  *contents = [fileManager contentsOfDirectoryAtPath:fileDirectory error:nil];

    for (fileContent in contents){
        NSString *fullFilePath = [fileDirectory stringByAppendingPathComponent:fileContent];
        WICED_BT_TRACE("  %s\n", fullFilePath.UTF8String);
        [fileManager removeItemAtPath:fullFilePath error: NULL];
    }
}


static CBPeripheral *currentConnectedPeripheral = nil;

+(void) setCurrentConnectedPeripheral:(CBPeripheral *)peripheral
{
    @synchronized (MeshNativeHelper.getSharedInstance) {
        currentConnectedPeripheral = peripheral;
    }
}

+(CBPeripheral *) getCurrentConnectedPeripheral
{
    CBPeripheral *peripheral = nil;
    @synchronized (MeshNativeHelper.getSharedInstance) {
        peripheral = currentConnectedPeripheral;
    }
    return peripheral;
}

#ifdef MESH_DFU_ENABLED
// DFU APIs
void mesh_client_dfu_status_cb(uint8_t state, uint8_t *p_data, uint32_t data_length)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_client_dfu_status_cb] state:0x%02x, data_length:%u\n", state, data_length);
    NSData *data = NULL;
    if (p_data == NULL || data_length == 0) {
        data = [NSData data];
    } else {
        data = [[NSData alloc] initWithBytes:p_data length:data_length];
    }
    [nativeCallbackDelegate meshClientDfuStatusCb:state data:data];
}

/*
 * when the interval is not set to 0, the DFU status will report in the DFU status in every internal time automatically.
 * if the interval is set to 0, the DFU status automatically report will be cancelled and stoped.
 */
+(int) meshClientDfuGetStatus:(int)interval
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientDfuGetStatus] interval=%d\n", interval);
    int ret;
    EnterCriticalSection();
    if (interval) {
        ret = mesh_client_dfu_get_status(mesh_client_dfu_status_cb, interval);
    } else {
        ret = mesh_client_dfu_get_status(NULL, 0);
    }
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientDfuStart:(int)dfuMethod firmwareId:(NSData *)firmwareId metadata:(NSData *)metadata
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientDfuStart] dfuMethod:%d\n", dfuMethod);
    wiced_bool_t self_distributor = (dfuMethod == DFU_METHOD_PROXY_TO_ALL) ? FALSE : TRUE;
    int ret;

    if (dfuMethod == DFU_METHOD_APP_TO_DEVICE) {
        WICED_BT_TRACE("[MeshNativeHelper meshClientDfuStart] error: DFU_METHOD_APP_TO_DEVIVEC does not using the mesh_client_dfu_start() API, using old OTA method\n");
        return MESH_CLIENT_ERR_METHOD_NOT_AVAIL;
    }

    EnterCriticalSection();
    ret = mesh_client_dfu_start((uint8_t *)firmwareId.bytes, (uint8_t)firmwareId.length, (uint8_t *)metadata.bytes, (uint8_t)metadata.length, TRUE, self_distributor);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientDfuStop
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientDfuStop]\n");
    int ret;
    EnterCriticalSection();
    ret = mesh_client_dfu_stop();
    LeaveCriticalSection();
    return ret;
}

// status 0 indicates finished on success; otherwise, non-zero indicates finished with some error.
+(void) meshClientDfuOtaFinish:(int)status
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientDfuOtaFinish] status:%d\n", status);
    EnterCriticalSection();
    mesh_client_dfu_ota_finish((uint8_t)status);
    LeaveCriticalSection();
}
#endif  // #ifdef MESH_DFU_ENABLED

// Sensor APIs
+(int) meshClientSensorCadenceGet:(NSString *)deviceName
                       propertyId:(int)propertyId
         fastCadencePeriodDivisor:(int *)fastCadencePeriodDivisor
                      triggerType:(int *)triggerType
                 triggerDeltaDown:(int *)triggerDeltaDown
                   triggerDeltaUp:(int *)triggerDeltaUp
                      minInterval:(int *)minInterval
                   fastCadenceLow:(int *)fastCadenceLow
                  fastCadenceHigh:(int *)fastCadenceHigh
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorCadenceGet] deviceName:%s, propertyId:0x%04x\n", deviceName.UTF8String, propertyId);
    uint16_t fast_cadence_period_divisor = 0;
    wiced_bool_t trigger_type = 0;
    uint32_t trigger_delta_down = 0;
    uint32_t trigger_delta_up = 0;
    uint32_t min_interval = 0;
    uint32_t fast_cadence_low = 0;
    uint32_t fast_cadence_high = 0;
    int ret;

    EnterCriticalSection();
    ret = mesh_client_sensor_cadence_get(deviceName.UTF8String, propertyId,
                                         &fast_cadence_period_divisor,
                                         &trigger_type,
                                         &trigger_delta_down,
                                         &trigger_delta_up,
                                         &min_interval,
                                         &fast_cadence_low,
                                         &fast_cadence_high);
    LeaveCriticalSection();

    if (ret == MESH_CLIENT_SUCCESS) {
        *fastCadencePeriodDivisor = (int)fast_cadence_period_divisor;
        *triggerType = (int)trigger_type;
        *triggerDeltaDown = (int)trigger_delta_down;
        *triggerDeltaUp = (int)trigger_delta_up;
        *minInterval = (int)min_interval;
        *fastCadenceLow = (int)fast_cadence_low;
        *fastCadenceHigh = (int)fast_cadence_high;
    }
    return ret;
}

+(int) meshClientSensorCadenceSet:(NSString *)deviceName
                       propertyId:(int)propertyId
         fastCadencePeriodDivisor:(int)fastCadencePeriodDivisor
                      triggerType:(int)triggerType
                 triggerDeltaDown:(int)triggerDeltaDown
                   triggerDeltaUp:(int)triggerDeltaUp
                      minInterval:(int)minInterval
                   fastCadenceLow:(int)fastCadenceLow
                  fastCadenceHigh:(int)fastCadenceHigh
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorCadenceSet] deviceName:%s, propertyId:0x%04x, %u, %u, %u, %u, %u, %u, %u\n",
          deviceName.UTF8String, propertyId, fastCadencePeriodDivisor, triggerType, triggerDeltaDown, triggerDeltaUp, minInterval, fastCadenceLow, fastCadenceHigh);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_sensor_cadence_set(deviceName.UTF8String, propertyId,
                                         (uint16_t)fastCadencePeriodDivisor,
                                         (wiced_bool_t)triggerType,
                                         (uint32_t)triggerDeltaDown,
                                         (uint32_t)triggerDeltaUp,
                                         (uint32_t)minInterval,
                                         (uint32_t)fastCadenceLow,
                                         (uint32_t)fastCadenceHigh);
    LeaveCriticalSection();
    return ret;
}

+(NSData *) meshClientSensorSettingGetPropertyIds:(NSString *)componentName
                                               propertyId:(int)propertyId
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorSettingGetPropertyIds] componentName:%s, propertyId:0x%04x\n", componentName.UTF8String, propertyId);
    NSData *settingPropertyIdsData = nil;
    int *settingPropertyIds = NULL;
    int count = 0;

    EnterCriticalSection();
    settingPropertyIds = mesh_client_sensor_setting_property_ids_get(componentName.UTF8String, propertyId);
    LeaveCriticalSection();

    if (settingPropertyIds != NULL) {
        while (settingPropertyIds[count] != WICED_BT_MESH_PROPERTY_UNKNOWN) {
            count += 1;
        }
        settingPropertyIdsData = [NSData dataWithBytes:(void *)settingPropertyIds length:(count * sizeof(int))];
        free(settingPropertyIds);
    }
    return settingPropertyIdsData;
}

+(NSData *) meshClientSensorPropertyListGet:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorPropertyListGet] componentName:%s\n", componentName.UTF8String);
    NSData *propertyListData = nil;
    int *propertyList = NULL;
    int count = 0;

    EnterCriticalSection();
    propertyList = mesh_client_sensor_property_list_get(componentName.UTF8String);
    LeaveCriticalSection();

    if (propertyList != NULL) {
        while (propertyList[count] != WICED_BT_MESH_PROPERTY_UNKNOWN) {
            count += 1;
        }
        propertyListData = [NSData dataWithBytes:(void *)propertyList length:(count * sizeof(int))];
        free(propertyList);
    }
    return propertyListData;
}

+(int) meshClientSensorSettingSet:(NSString *)componentName
                       propertyId:(int)propertyId
                settingPropertyId:(int)settingPropertyId
                            value:(NSData *)value
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorSettingSet] componentName:%s, propertyId:0x%04x, settingPropertyId:0x%04x\n", componentName.UTF8String, propertyId, settingPropertyId);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_sensor_setting_set(componentName.UTF8String, propertyId, settingPropertyId, (uint8_t *)value.bytes);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSensorGet:(NSString *)componentName propertyId:(int)propertyId
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSensorGet] componentName:%s, propertyId:0x%04x\n", componentName.UTF8String, propertyId);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_sensor_get(componentName.UTF8String, propertyId);
    LeaveCriticalSection();
    return ret;
}

+(BOOL) meshClientIsLightController:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientIsLightController] componentName:%s\n", componentName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_is_light_controller((char *)componentName.UTF8String);
    LeaveCriticalSection();
    return (ret == 0) ? FALSE : TRUE;
}

void meshClientLightLcModeStatusCb(const char *device_name, int mode)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientLightLcModeStatusCb] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    [nativeCallbackDelegate onLightLcModeStatusCb:[NSString stringWithUTF8String:(const char *)device_name]
                                             mode:mode];
}

+(int) meshClientGetLightLcMode:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetLightLcMode] componentName:%s\n", componentName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_mode_get(componentName.UTF8String, meshClientLightLcModeStatusCb);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSetLightLcMode:(NSString *)componentName mode:(int)mode
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetLightLcMode] componentName:%s, mode:%d\n", componentName.UTF8String, mode);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_mode_set(componentName.UTF8String, mode, meshClientLightLcModeStatusCb);
    LeaveCriticalSection();
    return ret;
}

void meshClientLightLcOccupancyModeStatusCb(const char *device_name, int mode)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientLightLcOccupancyModeStatusCb] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    [nativeCallbackDelegate onLightLcOccupancyModeStatusCb:[NSString stringWithUTF8String:(const char *)device_name]
                                                      mode:mode];
}

+(int) meshClientGetLightLcOccupancyMode:(NSString *)componentName
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetLightLcOccupancyMode] componentName:%s\n", componentName.UTF8String);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_occupancy_mode_get(componentName.UTF8String, meshClientLightLcOccupancyModeStatusCb);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSetLightLcOccupancyMode:(NSString *)componentName mode:(int)mode
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSetLightLcOccupancyMode] componentName:%s, mode:%d\n", componentName.UTF8String, mode);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_occupancy_mode_set(componentName.UTF8String, mode, meshClientLightLcOccupancyModeStatusCb);
    LeaveCriticalSection();
    return ret;
}


void meshClientLightLcPropertyStatusCb(const char *device_name, int property_id, int value)
{
    if (device_name == NULL || *device_name == '\0') {
        WICED_BT_TRACE("[MeshNativeHelper meshClientLightLcPropertyStatusCb] error: invalid parameters, device_name=0x%p\n", device_name);
        return;
    }
    WICED_BT_TRACE("[MeshNativeHelper meshClientLightLcPropertyStatusCb] device_name=%s, property_id=%d, value=%d\n", device_name, property_id, value);
    [nativeCallbackDelegate onLightLcPropertyStatusCb:[NSString stringWithUTF8String:(const char *)device_name]
                                           propertyId:property_id
                                                value:value];
}

+(int) meshClientGetLightLcProperty:(NSString *)componentName propertyId:(int)propertyId
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientGetLightLcProperty] componentName:%s, propertyId:0x%X\n", componentName.UTF8String, propertyId);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_property_get(componentName.UTF8String, propertyId, meshClientLightLcPropertyStatusCb);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSetLightLcProperty:(NSString *)componentName propertyId:(int)propertyId value:(int)value
{
    WICED_BT_TRACE("[MeshNativeHelper meshClientSetLightLcProperty] componentName:%s, propertyId:0x%X, value:0x%X\n", componentName.UTF8String, propertyId, value);
    int ret;
    EnterCriticalSection();
    ret = mesh_client_light_lc_property_set(componentName.UTF8String, propertyId, value, meshClientLightLcPropertyStatusCb);
    LeaveCriticalSection();
    return ret;
}

+(int) meshClientSetLightLcOnOffSet:(NSString *)componentName onoff:(uint8_t)onoff reliable:(BOOL)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay
{
    int ret;
    Boolean applied_reliable = [MeshNativeHelper meshClientIsGroup:componentName] ? false : reliable;  // for group, should always use unreliable mode.
    WICED_BT_TRACE("[MeshNativeHelper meshClientSetLightLcOnOffSet] componentName:%s, onoff:%d, reliable:%d, applied_reliable:%d\n", componentName.UTF8String, onoff, reliable, applied_reliable);
    EnterCriticalSection();
    ret = mesh_client_light_lc_on_off_set(componentName.UTF8String, onoff, applied_reliable, transitionTime, delay);
    LeaveCriticalSection();
    return ret;
}

+(BOOL) meshClientIsSameNodeElements:(NSString *)networkName elementName:(NSString *)elementName anotherElementName:(NSString *)anotherElementName
{
    BOOL isSameNode = FALSE;
    wiced_bt_mesh_db_mesh_t *pMeshDb = wiced_bt_mesh_db_init(networkName.UTF8String);
    if (pMeshDb == NULL) {
        return isSameNode;
    }

    wiced_bt_mesh_db_node_t *pMeshNode1 = wiced_bt_mesh_db_node_get_by_element_name(pMeshDb, elementName.UTF8String);
    wiced_bt_mesh_db_node_t *pMeshNode2 = wiced_bt_mesh_db_node_get_by_element_name(pMeshDb, anotherElementName.UTF8String);
    if ((pMeshNode1 == pMeshNode2) && (pMeshNode1 != NULL)) {
        isSameNode = TRUE;
    }

    wiced_bt_mesh_db_deinit(pMeshDb);
    return isSameNode;
}

+(int) meshClientGetNodeElements:(NSString *)networkName elementName:(NSString *)elementName
{
    int elements = 0;
    wiced_bt_mesh_db_mesh_t *pMeshDb = wiced_bt_mesh_db_init(networkName.UTF8String);
    if (pMeshDb == NULL) {
        return -ENFILE;
    }

    wiced_bt_mesh_db_node_t *pMeshNode = wiced_bt_mesh_db_node_get_by_element_name(pMeshDb, elementName.UTF8String);
    if (pMeshNode == NULL) {
        wiced_bt_mesh_db_deinit(pMeshDb);
        return -ENOENT;
    }
    elements = pMeshNode->num_elements;

    wiced_bt_mesh_db_deinit(pMeshDb);
    return elements;
}

+(int) meshClientIsNodeBlocked:(NSString *)networkName elementName:(NSString *)elementName
{
    BOOL isBlocked = FALSE;
    wiced_bt_mesh_db_mesh_t *pMeshDb = wiced_bt_mesh_db_init(networkName.UTF8String);
    if (pMeshDb == NULL) {
        return -ENFILE;
    }

    wiced_bt_mesh_db_node_t *pMeshNode = wiced_bt_mesh_db_node_get_by_element_name(pMeshDb, elementName.UTF8String);
    if (pMeshNode == NULL) {
        wiced_bt_mesh_db_deinit(pMeshDb);
        return -ENOENT;
    }
    isBlocked = pMeshNode->blocked ? TRUE : FALSE;

    wiced_bt_mesh_db_deinit(pMeshDb);
    return isBlocked;
}

static const uint32_t crc32Table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*
 * Help function to calculate CRC32 checksum for specific data.
 * Required by the mesh libraries which exists as a builtin function in the ROM.
 *
 * @param crc32     CRC32 value for calculating.
 *                  The initalize CRC value must be CRC32_INIT_VALUE.
 * @param data      Data required to calculate CRC32 checksum.
 *
 * @return          Calculated CRC32 checksum value.
 *
 * Note, after the latest data has been calculated, the final CRC32 checksum value must be calculuated as below as last step:
 *      crc32 ^= CRC32_INIT_VALUE
 */
uint32_t update_crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    uint32_t newCrc = crc;
    uint32_t n;

    for (n = 0; n < len; n++) {
        newCrc = crc32Table[(newCrc ^ buf[n]) & 0xFF] ^ (newCrc >> 8);
    }

    return newCrc;
}
+(uint32_t) updateCrc32:(uint32_t)crc data:(NSData *)data
{
    return update_crc32(crc, (uint8_t *)data.bytes, (uint32_t)data.length);
}

#ifdef MESH_DFU_ENABLED
+(void) meshClientSetDfuFwMetadata:(NSData *)fwId metadata:(NSData *)metadata
{
    dfuFwIdLen = (uint32_t)fwId.length;
    memcpy(dfuFwId, fwId.bytes, dfuFwIdLen);
    dfuMetadataDataLen = (uint32_t)metadata.length;
    memcpy(dfuMetadataData, metadata.bytes, dfuMetadataDataLen);
}

+(void) meshClientClearDfuFwMetadata
{
    mesh_dfu_metadata_init();
}

+(void) meshClientSetDfuFilePath:(NSString *)filePath
{
    char *dfuFilePath = (filePath == nil || filePath.length == 0) ? NULL : (char *)[[NSFileManager defaultManager] fileSystemRepresentationWithPath:filePath];
    setDfuFilePath(dfuFilePath);
}

+(NSString *) meshClientGetDfuFilePath
{
    char *filePath = getDfuFilePath();
    if (filePath == NULL) {
        return nil;
    }
    return [NSString stringWithUTF8String:(const char *)filePath];
}

void mesh_native_helper_read_dfu_metadata(uint8_t *p_fw_id, uint32_t *p_fw_id_len, uint8_t *p_metadata, uint32_t *p_metadata_len)
{
    *p_fw_id_len = dfuFwIdLen;
    *p_metadata_len = dfuMetadataDataLen;
    if (p_fw_id && dfuFwIdLen) {
        memcpy(p_fw_id, dfuFwId, dfuFwIdLen);
    }
    if (p_metadata && dfuMetadataDataLen) {
        memcpy(p_metadata, dfuMetadataData, dfuMetadataDataLen);
    }
}

uint32_t mesh_native_helper_read_file_size(const char *p_path)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *filePath = nil;
    unsigned long long fileSize = 0;

    if (p_path == NULL || !strlen(p_path)) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_size] error: input file path is NULL\n");
        return 0;
    }

    filePath = [NSString stringWithUTF8String:(const char *)p_path];
    if (![fileManager fileExistsAtPath:filePath] || ![fileManager isReadableFileAtPath:filePath]) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_size] error: file \"%s\" not exists or not readable\n", filePath.UTF8String);
        return 0;
    }

    fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:nil] fileSize];
    return (uint32_t)fileSize;
}

void mesh_native_helper_read_file_chunk(const char *p_path, uint8_t *p_data, uint32_t offset, uint16_t data_len)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *filePath = nil;
    NSFileHandle *handle = nil;
    NSData *data = nil;

    if (p_path == NULL || !strlen(p_path)) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_chunk] error: input file path is NULL\n");
        return;
    }

    filePath = [NSString stringWithUTF8String:(const char *)p_path];
    if (![fileManager fileExistsAtPath:filePath] || ![fileManager isReadableFileAtPath:filePath]) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_chunk] error: file \"%s\" not exists or not readable\n", filePath.UTF8String);
        return;
    }

    handle = [NSFileHandle fileHandleForReadingAtPath:filePath];
    if (handle == nil) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_chunk] error: unable to open file \"%s\" for reading\n", filePath.UTF8String);
        return;
    }

    @try {
        [handle seekToFileOffset:(unsigned long long)offset];
        data = [handle readDataOfLength:data_len];
        [data getBytes:p_data length:data.length];
    } @catch (NSException *exception) {
        WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_read_file_chunk] error: unable exception: %s\n", [[exception description] UTF8String]);
    } @finally {
        [handle closeFile];
    }
}
#endif  // #ifdef MESH_DFU_ENABLED

/* This API is implemented for test purpose only. It is used in the Scan Provision Test function. */
+(Boolean) isMeshClientProvisionKeyRefreshing
{
    typedef struct {
        /*
         * Note:
         *  Must make sure the the offset of the `state` variable and the value of the PROVISION_STATE_KEY_REFRESH_1 macro
         * are exactly same as the offset and value defined in the mesh_provision_cb_t data structure
         * in wiced_mesh_client.c file.
         */
        #define PROVISION_STATE_KEY_REFRESH_1   13
        uint8_t     state;
    } mesh_provision_cb_t;
    extern mesh_provision_cb_t provision_cb;
    Boolean isProvisionKeyRefreshing = false;

    EnterCriticalSection();
    if (provision_cb.state >= PROVISION_STATE_KEY_REFRESH_1) {
        isProvisionKeyRefreshing = true;
    }
    WICED_BT_TRACE("[MeshNativeHelper isMeshClientProvisionKeyRefreshing] p_provisioning_cb->state=%d, isProvisionKeyRefreshing=%d\n", provision_cb.state, isProvisionKeyRefreshing);
    LeaveCriticalSection();
    return isProvisionKeyRefreshing;
}

+(void) meshClientLogInit:(Boolean) is_console_enabled
{
    mesh_trace_log_init(is_console_enabled ? TRUE : FALSE);
}

+(void) meshClientLog:(NSString *)message
{
    if (message == nil || message.length == 0) {
        return;
    }

    Log("%s\n", message.UTF8String);
}

+(NSString *) meshClientGetNetworkName
{
    extern wiced_bt_mesh_db_mesh_t *p_mesh_db;
    if (p_mesh_db && p_mesh_db->name) {
        return [[NSString alloc] initWithUTF8String:p_mesh_db->name];
    }
    return nil;
}

#ifdef MESH_DFU_ENABLED
void mesh_native_helper_start_ota_transfer_for_dfu(void)
{
    WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_start_ota_transfer_for_dfu]\n");
    [nativeCallbackDelegate meshClientStartOtaTransferForDfu];
}

wiced_bool_t mesh_native_helper_is_ota_supported_for_dfu(void)
{
    wiced_bool_t is_ota_supported = [nativeCallbackDelegate meshClientIsOtaSupportedForDfu] ? WICED_FALSE : WICED_TRUE;
    WICED_BT_TRACE("[MeshNativeHelper mesh_native_helper_is_ota_supported_for_dfu] is_ota_supported=%d\n", is_ota_supported);
    return is_ota_supported;
}
#endif  // #ifdef MESH_DFU_ENABLED

@end
