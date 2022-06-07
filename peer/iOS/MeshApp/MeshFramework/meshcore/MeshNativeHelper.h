/*
 * Copyright 2016-2022, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * This file defines all APIs implemented in the MeshNativeHelper Objective-C class to wrap mesh libraries and functions.
 */

#ifndef MeshNativeHelper_h
#define MeshNativeHelper_h

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

typedef NS_ENUM(NSInteger, MeshClientNodeStatus) {
    MESH_CLIENT_NODE_WARNING_UNREACHABLE = 0,
    MESH_CLIENT_SUCCESS = 1,
    MESH_CLIENT_NODE_ERROR_UNREACHABLE = 2,
};

typedef NS_ENUM(NSInteger, MeshDfuEvent) {
    MESH_DFU_EVENT_START_OTA = 1,
};

#define CRC32_INIT_VALUE 0xFFFFFFFF

#define DFU_DISTRIBUTION_STATUS_TIMEOUT     10

#define MESH_DEVICE_DEFAULT_CONFIG_IS_GATT_PROXY                1
#define MESH_DEVICE_DEFAULT_CONFIG_IS_FRIEND                    1
#define MESH_DEVICE_DEFAULT_CONFIG_IS_RELAY                     1
#define MESH_DEVICE_DEFAULT_CONFIG_SEND_NET_BEACON              1
#define MESH_DEVICE_DEFAULT_CONFIG_RELAY_XMIT_COUNT             3
#define MESH_DEVICE_DEFAULT_CONFIG_RELAY_XMIT_INTERNAL          100
#define MESH_DEVICE_DEFAULT_CONFIG_DEFAULT_TTL                  8
#define MESH_DEVICE_DEFAULT_CONFIG_NET_XMIT_COUNT               3
#define MESH_DEVICE_DEFAULT_CONFIG_NET_XMIT_INTERNVAL           100
#define MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_CREDENTIAL_FLAG      0
#define MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_TTL                  8
#define MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_RETRANSMIT_COUNT     0
#define MESH_DEVICE_DEFAULT_CONFIG_PUBLISH_RETRANSMIT_INTERVAL  500

NS_ASSUME_NONNULL_BEGIN

@interface MeshNativeHelper : NSObject
+(MeshNativeHelper *) getSharedInstance;

/* @param delegate  pointer to the instance that implemented the IMeshNativeCallback protocol */
-(void) registerNativeCallback:(id) delegate;

#ifdef DEBUG    // for test purpose only, aimed to test system timer implementation.
-(uint32_t) meshStartTimer:(uint32_t)timeout type:(uint16_t)type;
-(void) meshStopTimer:(uint32_t)timerId;
-(uint32_t) meshRestartTimer:(uint32_t)timeout timerId:(uint32_t)timerId;
#endif

+(NSString *) getProvisionerUuidFileName;
+(int) setFileStorageAtPath:(NSString *)path;
+(int) setFileStorageAtPath:(NSString *)path provisionerUuid: (NSUUID * __nullable)provisionerUuid;
+(int) updateProvisionerUuid:(NSUUID * __nullable)provisionerUuid;
+(NSUUID *) generateRfcUuid;

+(int) meshClientNetworkExists:(NSString *)meshName;
+(int) meshClientNetworkCreate:(NSString *)provisionerName meshName:(NSString *)meshName;
+(int) meshClientNetworkOpen:(NSString *)provisionerName meshName:(NSString *)meshName;
+(int) meshClientNetworkDelete:(NSString *)provisionerName meshName:(NSString *)meshName;
+(void) meshClientNetworkClose;
+(NSString * __nullable) meshClientNetworkExport:(NSString *)meshName;
+(NSString * __nullable) meshClientNetworkImport:(NSString *)provisionerName jsonString:(NSString *)jsonString ifxJsonString:(NSString *)ifxJsonString;
+(int) meshClientGroupCreate:(NSString *)groupName parentGroupName:(NSString *)parentGroupName;
+(int) meshClientGroupDelete:(NSString *)groupName;
+(NSArray<NSString *> * __nullable) meshClientGetAllNetworks;
+(NSArray<NSString *> * __nullable) meshClientGetAllGroups:(NSString *)inGroup;
+(NSArray<NSString *> * __nullable) meshClientGetAllProvisioners;
+(NSArray<NSString *> * __nullable) meshClientGetDeviceComponents:(NSUUID *)uuid;
+(NSArray<NSString *> * __nullable) meshClientGetGroupComponents:(NSString *)groupName;
+(NSArray<NSString *> * __nullable) meshClientGetTargetMethods:(NSString *)componentName;
+(NSArray<NSString *> * __nullable) meshClientGetControlMethods:(NSString *)componentName;
+(Boolean) meshClientIsSubGroupName:(NSString *)componentName groupName:(NSString * __nullable)groupName;
+(Boolean) meshClientIsGroup:(NSString *)componentName;
+(uint8_t) meshClientGetComponentType:(NSString *)componentName;
+(int) meshClientRename:(NSString *)oldName newName:(NSString *)newName;
+(int) meshClientMoveComponentToGroup:(NSString *)componentName from:(NSString *)fromGroupName to:(NSString *)toGroupName;
+(int) meshClientConfigurePublication:(NSString *)componentName
                             isClient:(uint8_t)isClient
                               method:(NSString *)method
                           targetName:(NSString *)targetName
                        publishPeriod:(int)publishPeriod;
+(uint8_t) meshClientProvision:(NSString *)deviceName
                     groupName:(NSString *)groupName
                          uuid:(NSUUID *)uuid
              identifyDuration:(uint8_t)identifyDuration;
+(uint8_t) meshClientConnectNetwork:(uint8_t)useGattProxy scanDuration:(uint8_t)scanDuration;
+(uint8_t) meshClientDisconnectNetwork:(uint8_t)useGattProxy;

+(int) meshClientOnOffGet:(NSString *)deviceName;
+(int) meshClientOnOffSet:(NSString *)deviceName
                    onoff:(uint8_t)onoff
                 reliable:(Boolean)reliable     // set to true to get a reply ack; false without ack.
           transitionTime:(uint32_t)transitionTime
                    delay:(uint16_t)delay;
+(int) meshClientLevelGet:(NSString *)deviceName;
+(int) meshClientLevelSet:(NSString *)deviceName
                    level:(int16_t)level
                 reliable:(Boolean)reliable
           transitionTime:(uint32_t)transitionTime
                    delay:(uint16_t)delay;
+(int) meshClientHslGet:(NSString *)deviceName;
+(int) meshClientHslSet:(NSString *)deviceName
              lightness:(uint16_t)lightness
                    hue:(uint16_t)hue
             saturation:(uint16_t)saturation
               reliable:(Boolean)reliable
         transitionTime:(uint32_t)transitionTime
                  delay:(uint16_t)delay;
+(void) meshClientInit;
+(int) meshClientSetDeviceConfig:(NSString * __nullable)deviceName
                     isGattProxy:(int)isGattProxy
                        isFriend:(int)isFriend
                         isRelay:(int)isRelay
                          beacon:(int)beacon
                  relayXmitCount:(int)relayXmitCount
               relayXmitInterval:(int)relayXmitInterval
                      defaultTtl:(int)defaultTtl
                    netXmitCount:(int)netXmitCount
                 netXmitInterval:(int)netXmitInterval;
+(int) meshClientSetPublicationConfig:(int)publishCredentialFlag
               publishRetransmitCount:(int)publishRetransmitCount
            publishRetransmitInterval:(int)publishRetransmitInterval
                           publishTtl:(int)publishTtl;
+(int) meshClientResetDevice:(NSString *)componentName;
+(int) meshClientVendorDataSet:(NSString *)deviceName
                     companyId:(uint16_t)companyId
                       modelId:(uint16_t)modelId
                        opcode:(uint8_t)opcode
       disable_ntwk_retransmit:(uint8_t)disable_ntwk_retransmit
                          data:(NSData *)data;
+(int) meshClientIdentify:(NSString *)name duration:(uint8_t)duration;
+(int) meshClientLightnessGet:(NSString *)deviceName;
+(int) meshClientLightnessSet:(NSString *)deviceName lightness:(uint16_t)lightness reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay;
+(int) meshClientCtlGet:(NSString *)deviceName;
+(int) meshClientCtlSet:(NSString *)deviceName lightness:(uint16_t)lightness temperature:(uint16_t)temperature deltaUv:(uint16_t)deltaUv
               reliable:(Boolean)reliable transitionTime:(uint32_t)transitionTime delay:(uint16_t)delay;

//MESH CLIENT GATT APIS
+(void) meshClientScanUnprovisioned:(int)start uuid:(NSData * __nullable)uuid;
+(Boolean) meshClientIsConnectingProvisioning;
+(void) meshClientConnectionStateChanged:(uint16_t)connId mtu:(uint16_t)mtu;
+(void) meshClientAdvertReport:(NSData *)bdaddr addrType:(uint8_t)addrType rssi:(int8_t)rssi advData:(NSData *) advData;
+(uint8_t) meshConnectComponent:(NSString *)componentName useProxy:(uint8_t)useProxy scanDuration:(uint8_t)scanDuration;

+(void) sendRxProxyPktToCore:(NSData *)data;
+(void) sendRxProvisPktToCore:(NSData *)data;

+(Boolean) isMeshProvisioningServiceAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData;
+(Boolean) isMeshProxyServiceAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData;
+(Boolean) isMeshAdvertisementData:(NSDictionary<NSString *,id> *)advertisementData;
+(NSData *)peripheralIdentifyToBdAddr:(CBPeripheral *)peripheral;
+(NSData * __nullable) getConvertedRawMeshAdvertisementData:(CBPeripheral *)peripheral
                                          advertisementData:(NSDictionary<NSString *,id> *)advertisementData
                                                       rssi:(NSNumber *)rssi;
+(void) meshClientSetGattMtu:(int)mtu;
+(Boolean) meshClientIsConnectedToNetwork;
+(int) meshClientAddComponent:(NSString *)componentName toGorup:(NSString *)groupName;
+(NSArray<NSString *> *) meshClientGetComponentGroupList:(NSString *)componentName;
+(int) meshClientRemoveComponent:(NSString *)componentName from:(NSString *)groupName;
+(uint8_t) meshClientGetComponentInfo:(NSString *)componentName;

+(int) meshClientListenForAppGroupBroadcasts:(NSString * __nullable)controlMethod groupName:(NSString * __nullable)groupName startListen:(BOOL)startListen;
+(NSString *) meshClientGetPublicationTarget:(NSString *)componentName isClient:(BOOL)isClient method:(NSString *)method;
+(int) meshClientGetPublicationPeriod:(NSString *)componentName isClient:(BOOL)isClient method:(NSString *)method;

// OTA crypt helpers, return nil on error.
+(NSData * __nullable) meshClientOTADataEncrypt:(NSString *)componentName data:(NSData *)data;
+(NSData * __nullable) meshClientOTADataDecrypt:(NSString *)componentName data:(NSData *)data;

#ifdef MESH_DFU_ENABLED
// DFU APIs
+(int) meshClientDfuGetStatus:(int)interval;
+(int) meshClientDfuStart:(int)dfuMethod firmwareId:(NSData *)firmwareId metadata:(NSData *)metadata;
+(int) meshClientDfuStop;
+(void) meshClientDfuOtaFinish:(int)status;
#endif

// Sensor APIs
+(int) meshClientSensorCadenceGet:(NSString *)deviceName
                       propertyId:(int)propertyId
         fastCadencePeriodDivisor:(int *)fastCadencePeriodDivisor
                      triggerType:(int *)triggerType
                 triggerDeltaDown:(int *)triggerDeltaDown
                   triggerDeltaUp:(int *)triggerDeltaUp
                      minInterval:(int *)minInterval
                   fastCadenceLow:(int *)fastCadenceLow
                  fastCadenceHigh:(int *)fastCadenceHigh;
+(int) meshClientSensorCadenceSet:(NSString *)deviceName
                       propertyId:(int)propertyId
         fastCadencePeriodDivisor:(int)fastCadencePeriodDivisor
                      triggerType:(int)triggerType
                 triggerDeltaDown:(int)triggerDeltaDown
                   triggerDeltaUp:(int)triggerDeltaUp
                      minInterval:(int)minInterval
                   fastCadenceLow:(int)fastCadenceLow
                  fastCadenceHigh:(int)fastCadenceHigh;
+(NSData * __nullable) meshClientSensorSettingGetPropertyIds:(NSString *)componentName
                                                     propertyId:(int)propertyId;
+(NSData * __nullable) meshClientSensorPropertyListGet:(NSString *)componentName;
+(int) meshClientSensorSettingSet:(NSString *)componentName
                       propertyId:(int)propertyId
                settingPropertyId:(int)settingPropertyId
                            value:(NSData *)value;
+(int) meshClientSensorGet:(NSString *)componentName propertyId:(int)propertyId;

// Light LC APIs
+(BOOL) meshClientIsLightController:(NSString *)componentName;
+(int) meshClientGetLightLcMode:(NSString *)componentName;
+(int) meshClientSetLightLcMode:(NSString *)componentName mode:(int)mode;
+(int) meshClientGetLightLcOccupancyMode:(NSString *)componentName;
+(int) meshClientSetLightLcOccupancyMode:(NSString *)componentName mode:(int)mode;
+(int) meshClientGetLightLcProperty:(NSString *)componentName propertyId:(int)propertyId;
+(int) meshClientSetLightLcProperty:(NSString *)componentName propertyId:(int)propertyId value:(int)value;
+(int) meshClientSetLightLcOnOffSet:(NSString *)componentName
                              onoff:(uint8_t)onoff
                           reliable:(BOOL)reliable
                     transitionTime:(uint32_t)transitionTime
                              delay:(uint16_t)delay;

// Help APIs
+(void) meshBdAddrDictAppend:(NSData *)bdAddr peripheral:(CBPeripheral *)peripheral;
+(CBPeripheral * __nullable) meshBdAddrDictGetCBPeripheral:(NSData *)bdAddr;
+(void) meshBdAddrDictDelete:(NSData *)bdAddr;
+(void) meshBdAddrDictDeleteByCBPeripheral:(CBPeripheral *)peripheral;
+(void) meshBdAddrDictClear;
-(NSString*)getJsonFilePath;
- (void) destoryMeshClient;
+(NSData *) getMeshPeripheralMappedBdAddr:(CBPeripheral *)peripheral;

+(void) setCurrentConnectedPeripheral:(CBPeripheral * __nullable)peripheral;
+(CBPeripheral * __nullable) getCurrentConnectedPeripheral;

+(NSData *) MD5:(NSData *)data;
+(BOOL) meshClientIsSameNodeElements:(NSString *)networkName elementName:(NSString *)elementName anotherElementName:(NSString *)anotherElementName;
+(int) meshClientGetNodeElements:(NSString *)networkName elementName:(NSString *)elementName;
+(int) meshClientIsNodeBlocked:(NSString *)networkName elementName:(NSString *)elementName;

+(uint32_t) updateCrc32:(uint32_t)crc data:(NSData *)data;

#ifdef MESH_DFU_ENABLED
+(void) meshClientSetDfuFwMetadata:(NSData *)fwId metadata:(NSData *)metadata;
+(void) meshClientClearDfuFwMetadata;
+(void) meshClientSetDfuFilePath:(NSString * __nullable)filePath;
+(NSString *) meshClientGetDfuFilePath;
#endif

+(Boolean) isMeshClientProvisionKeyRefreshing;
+(void) meshClientLog:(NSString *)message;
+(void) meshClientLogInit:(Boolean) is_console_enabled;
+(NSString * __nullable) meshClientGetNetworkName;
@end

NS_ASSUME_NONNULL_END

#endif  /* MeshNativeHelper_h */
