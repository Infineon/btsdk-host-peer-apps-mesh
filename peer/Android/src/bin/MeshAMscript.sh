adb shell am startservice com.cypress.le.mesh.meshapp/.MeshAm
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.initialize
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.create.network --es name "newnetwork" --es provName "myprov"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.open.network --es name "newnetwork" --es provName "myprov"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.create.group --es groupName "mygroup" --es parentGroupName "newnetwork"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.groups --es inGroup "newnetwork"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.scan.mesh.devices.start  --es uuid "ff00ff00-ff00-ff00-ff00-ff00ff00ff00" --ei duration 1000
sleep 10
adb shell am broadcast -a com.cypress.le.mesh.meshapp.scan.mesh.devices.stop
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.is.network.exist --es meshName "newnetwork" --ei useGattProxy 1
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.target.methods --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.provision --es groupName "mygroup" --es deviceName "mylight" --es identity 10 --es uuid "ff00ff00-ff00-ff00-ff00-ff00ff00ff00"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.connect.network --ei transport 1 --ei scanDuration 100
sleep 4
adb shell am broadcast -a com.cypress.le.mesh.meshapp.onoff.set --es componentName "mylight\ \(0002\)" --ez onOff true --ez reliable true --ei delay 0 --ei transitionTime 0
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.hsl.set --es componentName "mylight\ \(0002\)" --ei hue 15 --ei saturation 20 --ei lightness 100 --ez reliable true --ei delay 0 --ei transitionTime 0
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.hsl.get --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.lightness.set --es componentName "mylight\ \(0002\)" --ei lightness 100 --ez reliable true --ei delay 0 --ei transitionTime 0
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.lightness.get --es componentName "mylight\ \(0002\)"
sleep 4
adb shell am broadcast -a com.cypress.le.mesh.meshapp.level.set --es componentName "mylight\ \(0002\)" --ei level 11 --ez reliable true --ei delay 0 --ei transitionTime 0
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.level.get --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.ctl.set --es componentName "mylight\ \(0002\)" --ei temperature 7773 --ei deltaUv 0 --ei lightness 32781 --ez onOff true --ez reliable true --ei delay 0 --ei transitionTime 0
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.ctl.get --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.provisioners
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.device.components --es uuid "ff00ff00-ff00-ff00-ff00-ff00ff00ff00"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.target.methods --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.control.methods --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.export.network --es networkName "newnetwork"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.is.network.exist --es meshName "newnetwork"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.disconnect.network --ei transport 1
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.connect.component --es componentName "mylight\ \(0002\)" --ei useGATTProxy 1 --ei scanDuration 1000
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.start.ota.upgrade --es componentName "mylight\ \(0002\)" --es fileName "/sdcard/download/ota.bin"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.delete.group --es groupName "mygroup123"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.delete.network --es provisionerName "myprov" --es meshName "ISO"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.move.component.to.group --es componentName "mylight\ \(0002\)" --es groupName "mygroup123"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.group.components --es groupName "mygroup123"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.close.network
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.import.network --es provName "myprov" --es filepath "/sdcard/exports/newnetwork.json"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.set.publication.config --es deviceName "mylight\ \(0002\)" --ei deviceType 0 --ei publishPeriod 10 --ei publishCredentialFlag 0 --ei publishRetransmitCount 0 --ei publishRetransmitInterval 500 --ei publishTtl 63
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.set.device.config --es deviceName "mylight\ \(0002\)" --ez isGattProxy true --ez isFriend true --ez isRelay true --ei relayXmitCount 3 --ei relayXmitInterval 100 --ei defaultTtl 63 --ei netXmitCount 3 --ei netXmitInterval 100
sleep 2
#adb shell am broadcast -a com.cypress.le.mesh.meshapp.identify --es name "mylight\ \(0002\)" --ei duration 100
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.rename --es oldName "mylight\ \(0002\)" --es newName "mylight"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.networks
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.all.provisioners
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.component.type --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.component.type --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.stop.ota.upgrade
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.current.network
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.is.connected.to.network
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.start.tracking
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.stop.tracking
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.uninitialize
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.get.component.info --es componentName "mylight\ \(0002\)"
sleep 2
adb shell am broadcast -a com.cypress.le.mesh.meshapp.reset.device --es componentName "mylight\ \(0002\)"
