
Installation, Usage, and Public APIs
----------------------------------
1>  Install MeshLightingController.apk on an Android phone.
    Grant permission when the app opens for the first time.
     (The preferred version would be Android 7.0 or 7.1.1)
2>  The public APIs are documented in the class MeshController.java. Developers can also generate an API document using Android studio.


Using the MeshLightingController, create a network.
Create a room.

Adding Light:
-------------
1>  Create the WICED application mesh_onoff_server/mesh_light_hsl_server (light).
2>  Adding Lights to the room:
    a> Add lights inside the room.
    b> The application wraps provisioning and configuration as part of adding a light.
    Note: it might take about 10-20 seconds to add a light.
    c> Once provisioning and configuration are completed, a 'provision complete' popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added, an appropriate UI option (OnOff/HSL) will appear.
4>  To do device-specific operations, click on the device and use the controls.

Adding Temperature Sensor:
--------------------------
1>  Create the WICED application sensor_temperature.
2>  Adding a sensor to the room:
    a> Add a sensor inside the room.
    b> The application wraps provisioning and configuration as part of adding the sensor.
    Note: it might take about 10-20 seconds to add a sensor.
    c> Once provisioning and configuration are completed, a 'provision complete' popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added, an appropriate UI option (sensor) will appear.
4>  Configuration of the sensor:
    a> Select the property of the sensor to control/configure. (Currently, only temperature sensor is supported).
    b> Click on "Configure", then you can configure sensor publication, cadence, and settings.
    c> To get the current sensor data, click on "Get Sensor Data".
    d> Set the cadence of the sensor:
        Set the minimum interval in which sensor data will be published.
        Set the range in which the fast cadence will be observed.
        Set the fast cadence period (how fast the data will be published with respect to the publishing period).
        Set the unit in which if the values change the data should be published, and trigger type (Native or percentage)
          example: publish data if the data changes by 2 units/10%

Adding Switch:
--------------
1>  Install the WICED application mesh_onoff_client (switch).
2>  Adding Switches to the room:
    a>  Add switches inside the room.
    b>  The application wraps provisioning and configuration as part of adding a switch.
    Note: it might take about 10-20 seconds to add a switch.
    c> Once provisioning and configuration are completed, a 'provision complete' popup will appear and the appropriate device will be seen on the UI.
3>  Depending on the type of device added, an appropriate UI option (Switch) will appear.
4>  To do device-specific operations, click on the device and use the controls.
5>  To assign a light to a switch:
    a> Click on the assign button on the switch. Select the appropriate light from the popup.
       The light selected will be assigned to the switch.
    b> To use the switch to control the light using the ClientControlMesh host app:
       Select the Models tab. Select "onoff" from the dropdown.
       Select "use publication info" and "Reliable" checkbox.
       Set an appropriate on/off state and click on set. The light should respond appropriately.

Note: Ideally a real switch would use a button on the board to send toggle onoff messages.
      mesh_onoff_client app can be modified to have the button pressed event mapped to sending mesh_onoff_client_set.
      Hint: Refer to the ClientControlMesh source code to form the packet.


Added Mesh OTA support
----------------------
1> On the UI if the user clicks on any of the added devices users will find an option to upgrade OTA.
2> Store the OTA file to the phone and provide the path to the OTA file.
3> Create a Mesh OTA file using the appropriate Mesh app using WICED SDK.
For example, when the user builds the mesh_onoff_server application, a binary file is located in the build directory.
For the mesh_onoff_server app, the file in the build directory would be named "mesh_onoff_server-<name>.ota.bin"

Mesh database JSON export/Import
--------------------------------
Cypress Mesh Controller framework stores Mesh network information in .json file format specified by SIG MESH WG.
1> During Provisioning Android Mesh Controller stores the database in the application's internal memory.
2> To exercise a use case such as control of Mesh devices using multiple phones follow these steps:
	a> After creating a network and provisioning a few devices on phone P1:
       Use the option "export network" in the home screen settings to export the required Mesh database.
    b> The Cypress Mesh Lighting app stores the exported Mesh database in "/sdcard/exports" directory.
    c> A user can now move the exported file to another phone (Say phone P2).
    d> Install the Mesh Lighting app on P2. Use the "import network" option available in the settings menu of the main screen.
    e> The user can now control the Mesh devices using P2.

For more information refer to the public APIs (importNetwork/exportNetwork) in MeshController.java

Support added to control Mesh devices through the cloud via Cypress Mesh gateway
--------------------------------------------------------------------
Cypress Mesh solutions support the Mesh Gateway application which runs on combo chips. The current SDK consists of a Wi-Fi app named "bt_internet_gateway/mesh_network"
Please look at the app notes to see how to set up a gateway.

Note:
1. "bt_internet_gateway/mesh_network" is supported only on CYW943907WAE3 board.
2. Currently, the Android app supports only REST transport, the existing Android library for MQTT through AWS needs to be updated to
the latest AWS SDK needs to be updated and this will be done in the next release.

Below are the instructions to set up the gateway using the Android application.
1> User can choose to use REST or MQTT as the IoT protocol to send Mesh data to the gateway.
2> The choice of protocol is configurable in the gateway app (Please read the app note for the "bt_internet_gateway/mesh_network" app).
3> If the user chooses to use MQTT via AWS cloud, then the MeshLightingController expects AWS credentials to be provided in the AWS.conf file placed in the /sdcard
   directory. An example AWS.conf is provided in the current directory.
4> To add a Mesh gateway, go to the Home screen use setting option and click on "Add BT Internet Gateway".
5> If the Mesh gateway is in the unprovisioned state, the user can see the gateway advertising with the name "mesh proxy" select the device.
6> If the chosen transport is REST, then the user is expected to key-in the IP address of the gateway and also ensure that the phone and the
   gateway are connected to the same Wi-Fi AP (IP address of gateway should be available in gateway's console window).
7> If the chosen transport is MQTT over AWS IoT, then the Android app uses the credentials provided in the AWS.conf file. It's important to ensure that
   the gateway and phone have Internet connectivity.
   Note: try not to use an office network as they may block ports related to MQTT/AWS/...etc and hence it's advised to use personal hotspots.
8> After provisioning the gateway, the user can exercise HOME/AWAY use cases.
9> HOME mode: The app is connected to one of the proxy devices in the home and can control all the devices. By default, the app is always set to HOME.
   AWAY mode: The app is connected to a cloud/AP and the phone sends Mesh data to a specific device via the gateway.
10> To go to AWAY mode, use the setting option in the home screen and select the "go-to Away" option. This will disconnect the proxy connection and the phone connects
   to Cloud/AP.


Known Issue
-------------
Sometimes the GATT connection fails on Android and the log shows GATT 133 connection error
This issue is under debug in the Android community.
