#-------------------------------------------------
#
# Project created by QtCreator 2013-07-01T18:28:57
#
#-------------------------------------------------

QT       += core gui
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


# build configuration to build in customer repo downloaded from github
CONFIG += mtb_release

# build for WICED SDK, use UART to talk to embedded app
#CONFIG += wiced

# build for BSA, use local socket to talk to local BSA server, no support for UART
#CONFIG += bsa

TARGET = mesh_client
TEMPLATE = app
//DEFINES += MESH_AUTOMATION_ENABLED

SOURCES += main.cpp mainwindow.cpp serial_port.cpp \
    sensorcfgdlg.cpp

INCLUDEPATH += .
QMAKE_CXXFLAGS += -fpermissive
QMAKE_CFLAGS += -fpermissive

wiced:!macx:unix {
    message("wiced build linux")

    SOURCES += ../../../../../test/automation/nanopb/mesh_client.pb.c \
    ../../../../../test/automation/nanopb/pb_common.c \
    ../../../../../test/automation/nanopb/pb_decode.c \
    ../../../../../test/automation/nanopb/pb_encode.c \
    ../../../../../test/automation/nanopb/rpc.pb.c \
    ../peerapps/Windows/MeshClient/automation/mesh_client_script.cpp \
    ../peerapps/Windows/MeshClient/automation/mesh_client_rpc.c \
    ../peerapps/Windows/MeshClient/automation/script_app.c \
    src/ThreadHelper.cpp \
    src/SocketHelper.cpp

    INCLUDEPATH += . \
    ../peerapps/Windows/MeshClient/automation \
    ../../../../../test/automation/nanopb \
    ../../../../libraries/mesh_core_lib
#    ../../../../../20719-B1_Bluetooth/include/20719/hal
}

wiced {
    SOURCES += ../../../../libraries/mesh_client_lib/wiced_mesh_client.c
    SOURCES += ../../../../libraries/mesh_client_lib/wiced_bt_mesh_db.c
    SOURCES += ../../../../libraries/mesh_client_lib/wiced_mesh_api.c
    SOURCES += ../../../../libraries/mesh_client_lib/meshdb.c

    DEFINES += PROVISION_SCAN_REPORT_INCLUDE_BDADDR

    INCLUDEPATH += .
    INCLUDEPATH += $$_PRO_FILE_PWD_/.
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../include
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../libraries/mesh_client_lib
    INCLUDEPATH += $$_PRO_FILE_PWD_/include
}

mtb_release {
    SOURCES += ../../mesh_client_lib/wiced_mesh_client.c
    SOURCES += ../../mesh_client_lib/wiced_bt_mesh_db.c
    SOURCES += ../../mesh_client_lib/wiced_mesh_api.c
    SOURCES += ../../mesh_client_lib/meshdb.c

    DEFINES += PROVISION_SCAN_REPORT_INCLUDE_BDADDR

    INCLUDEPATH += .
    INCLUDEPATH += $$_PRO_FILE_PWD_/.
    INCLUDEPATH += $$_PRO_FILE_PWD_/include
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../dev-kit/baselib/20819A1/include
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../dev-kit/baselib/20819A1/include/internal
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../dev-kit/baselib/20819A1/include/hal
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../dev-kit/baselib/20819A1/include/stack
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../../../dev-kit/btsdk-include
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../include
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../mesh_client_lib
}

wiced:macx:win32:win64 {
    message("wiced build")

}

bsa {
    message("BSA build")
    DEFINES += BSA
    DEFINES += __ANDROID__
    INCLUDEPATH += ./include
    SOURCES += app_manager.c
    SOURCES += app_mesh.c
    SOURCES += ../../../brcm/bsa/server/mesh/common/libraries/mesh_client_lib/wiced_mesh_api.c
    SOURCES += ../../../brcm/bsa/server/mesh/common/libraries/mesh_client_lib/wiced_bt_mesh_db.c
    SOURCES += ../../../brcm/bsa/server/mesh/common/libraries/mesh_client_lib/meshdb.c
    SOURCES += ../../../brcm/bsa/server/mesh/common/libraries/mesh_client_lib/wiced_mesh_client.c

    BSA_PATH = ../../../brcm/bsa

    INCLUDEPATH += ../../../../include
    INCLUDEPATH += ../../../../../../3rdparty/embedded/bsa_examples/linux/app_common/include
    INCLUDEPATH += ../../../../../../3rdparty/embedded/bsa_examples/linux/libbsa/include
    INCLUDEPATH += ../libbsa/include
    INCLUDEPATH += ../../../brcm/bsa/include
    INCLUDEPATH += ../../../bsa_examples/linux/app_common/include
    INCLUDEPATH += ../../../bsa_examples/linux/libbsa/include
    INCLUDEPATH += $$BSA_PATH/server/mesh/common/libraries/mesh_client_lib
    INCLUDEPATH += $$BSA_PATH/server/mesh/common/include
    INCLUDEPATH += $$BSA_PATH/server/mesh/20719-B1_Bluetooth/include/20719/
    INCLUDEPATH += $$BSA_PATH/server/mesh/20719-B1_Bluetooth/include/20719/hal
    INCLUDEPATH += $$BSA_PATH/20719-B1_Bluetooth/include/20719/stack
    LIBS += -L../libbsa/build/x86_64 -lbsa
}

HEADERS  += include/data_types.h \
            include/add_defines.h \
            mainwindow.h \
            ../../../../libraries/mesh_client_lib/wiced_mesh_client.h \
            ../../../../libraries/mesh_client_lib/meshdb.h \
            ../../../../libraries/mesh_client_lib/wiced_bt_mesh_db.h \
            include/win_data_types.h \
    sensorcfgdlg.h

FORMS    += mainwindow.ui \
    sensorcfgdlg.ui

DEFINES += QT_APP
DEFINES += CLIENTCONTROL
DEFINES += BLE_INCLUDED
DEFINES += WICEDX_LINUX

unix {
    SOURCES += btspy_ux.c
}

win32 {
    DEFINES += __windows__
    SOURCES += btspy_win32.c

    contains(QT_ARCH, x86_64) {
        # 64-bit windows
        LIBS += $$_PRO_FILE_PWD_\lib64\ws2_32.lib
    } else {
        # 32-bit windows
        LIBS += $$_PRO_FILE_PWD_\lib32\ws2_32.lib
    }
}
