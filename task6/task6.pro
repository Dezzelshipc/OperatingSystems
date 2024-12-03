QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += core \
    widgets \
    network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Network/clientmanager.cpp \
    Utility/config.cpp \
    Widgets/aboutwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    Network/client.cpp \
    Utility/config.cpp


HEADERS += \
    Network/clientmanager.h \
    Utility/config.h \
    Widgets/aboutwidget.h \
    mainwindow.h \
    Network/client.h \
    Utility/config.h


FORMS += \
    Widgets/aboutwidget.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TRANSLATIONS += \
    Localization/task6_en_US.ts \
    Localization/task6_ru_RU.ts
