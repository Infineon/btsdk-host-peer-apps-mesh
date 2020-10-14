# btsdk-host-peer-apps-mesh

### Overview

This repo contains host and peer apps to be used with embedded Mesh apps. Host apps act as an MCU to demonstrate use of the WICED HCI protocol to control embedded apps. Peer apps demonstrate Mesh configuration and provisioning from other peer Mesh devices.
Binary and source code is included.

Repo contents:

* host/VS\_ClientControl: Host MCU app that can provision and control an embedded Mesh app. It provides a UI to control all Mesh models. Supported OS - Windows.

* host/QT\_ClientControl: Host MCU app that can provision and control an embedded Mesh app for the Mesh Lighting Model. Supported OS - Windows, Linux, and macOS.

* peer/: Peer Mesh device app that demonstrates Mesh configuration and provisioning of an embedded app from a peer device. Supported OS: Android, iOS, WatchOS, and Windows.

* mesh\_client\_lib/: Mesh client support library used by both host and peer apps.
