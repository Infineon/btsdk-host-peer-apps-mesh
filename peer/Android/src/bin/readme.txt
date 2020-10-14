
Installation, Usage, and Public APIs
----------------------------------
1>  Install MeshLightingController.apk on an Android phone.
    Grant permission when the app opens for the first time.
     (The preferred version would be Android 7.0 or 7.1.1)
2>  To public APIs are documented in the class MeshController.java. Developers can also generate an API document using Android studio.


Using the MeshLightingController, create a network.
Create a room.

Adding Light:
-------------
1>  Create the wiced application mesh_onoff_server/mesh_light_hsl_server(light)
2>  Adding Lights to the room
    a> Add lights inside room
    b> The application wraps provisioning and configuration as part of adding light
    Note: it might take about 10-20 seconds to add a light
    c> Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added appropriate UI option (OnOff/HSL) will appear
4>  To do device-specific operation click on the device and use the controls.

Adding Temperature Sensor:
--------------------------
1>  Create the wiced application sensor_temperature
2>  Adding sensor to the room
    a> Add sensor inside room
    b> The application wraps provisioning and configuration as part of adding sensor
    Note: it might take about 10-20 seconds to add a sensor
    c> Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added appropriate UI option (sensor) will appear
4>  Configuration of sensor:
    a> Select the property of the sensor to control/configure. (Currently, only temperature sensor is supported)
    b> Click on "Configure", now you can configure sensor publication, cadence, and settings.
    c> To get the current sensor data, click on "Get Sensor Data".
    d> set the cadence of the sensor :
        set minimum interval in which sensor data has to be published.
        set the range in which the fast cadence has to be observed.
        set the fast cadence period (how fast the data has to be published with respect to publishing period).
        set the unit in which if the values change the data should be published and trigger type (Native or percentage), example: publish data if the data changes by 2 units/10%

Adding Switch:
--------------
1>  Install the wiced application mesh onoff_client (switch)
2>  Adding Switches to the room
    a>  Add switches inside the room
    b>  The application wraps provisioning and configuration as part of adding the switch
    Note: it might take about 10-20 seconds to add a switch
    c> Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added appropriate UI option (Switch) for the control will appear
4>  To do device-specific operation click on the device and use the controls.
5>  To assign light to a switch :
    a> Click on the assign button on the switch. Select appropriate light from the Popup.
       Light selected will be assigned to the switch.
    b> To use the switch to control light using mesh client control.
       Select the Models tab. Select "onoff" from the dropdown.
       Select "use publication info" and "Reliable" checkbox.
       Set an appropriate on/off state and click on set. The light should respond appropriately.

Note: Ideally a real switch would use a button on the board to send toggle onoff messages.
      mesh_onoff_client app can be modified to have button pressed event mapped to sending mesh_onoff_client_set
      Hint: Refer to client control code to form the packet


Added Mesh OTA support
----------------------
1> on the UI if the user clicks on any of the added device users will find an option to upgrade OTA
2> store the ota file to the phone and provide the path to the ota file
3> Create mesh OTA file using the appropriate mesh app in SDK ,
for example, when the user builds onOff server application using wiced SDK , a binary file is located in the build directory
for OnOffServer the file in the build directory would be named as "mesh_onoff_server-<name>.ota.bin"

Mesh database JSON export/Import
--------------------------------
Cypress Mesh Controller framework stores mesh network information in .json file format specified by SIG MESH WG
1> During Provisioning Android Mesh Controller stores the database in the application's internal memory
2> To exercise use case such as control of mesh devices using multiple phones follow the below steps
	a> After creating a network and provisioning few devices on phone P1
          use the option "export network" in the settings of the home screen to export the required meshdb.
        b> Cypress mesh lighting app stores the exported Meshdb in "/sdcard/exports" directory.
        c> A user can now move the exported file to another phone (Say phone P2)
        d> Install MeshLighting app on P2. Use the "import network" option available in the settings menu of the main screen.
        e> The user can now control the mesh devices using P2.

for more information refer to the public APIs (importNetwork/exportNetwork) in MeshController.java

Support added to control mesh devices through the cloud via Cypress mesh gateway
--------------------------------------------------------------------
Cypress mesh solution supports the mesh Gateway application which runs on combo chips, the current SDK consists of a wifi app named "bt_internet_gateway/mesh_network"
Please look at the app notes to set up a gateway.

Note :
1. "bt_internet_gateway/mesh_network" is supported only on CYW943907WAE3 board.
2. At the moment the Android app supports only REST transport, the existing android library for MQTT through AWS needs to be updated to
the latest AWS SDK needs to be updated and this will be done in the next release.

Below are the instructions to set up the gateway using the android application.
1> User can choose to use REST or MQTT as the IoT protocol to send mesh data to the gateway.
2> The choice of protocol is configurable in the gateway app (Please read the app note of "bt_internet_gateway/mesh_network" app)
3> If the user chooses to use MQTT via AWS cloud then, MeshLightingController expects AWS credentials to be provided in the AWS.conf file placed in /sdcard
directory. An example AWS.conf is provided in the current directory.
4> To add a mesh gateway, go to the Home screen use setting option and click on "Add BT Internet Gateway"
5> If the mesh gateway is in the unprovisioned state, the user can see the gateway advertising with the name "mesh proxy" select the device.
6> If the chosen transport is REST, then the user is expected to key-in the IP address of the gateway and also ensure that the phone and the
   gateway is connected to the same WiFi AP (IP address of gateway should be available in gateway's console window)
7> If the chosen transport is MQTT over AWS IoT, then the Android app uses the credentials provided in the AWS.conf file. It's important to ensure that
   the gateway and phone have internet connectivity.
Note: try not to use office network as they may block ports related to MQTT/AWS/...etc and hence it's advised to use personal hotspots.
8> After provisioning the gateway, the user can exercise HOME/AWAY use cases.
9> HOME mode: The app is connected to one of the proxy devices in the home and can control all the devices. By default, app is always set to HOME.
   AWAY mode: The app is connected to cloud/AP and the phone sends mesh data to a specific device via the gateway.
10> To go to AWAY mode, use the setting option in the home screen and select the "go-to Away" option. This will disconnect the proxy connection and phone connects
   to Cloud/AP.


Known Issue
-------------
Sometime Gatt connection fails on android and the log shows GATT 133 connection error
This issue is under debug in the Android community.
