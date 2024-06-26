QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    positions_truct.cpp \
    board.cpp \
    search.cpp \
    square.cpp

HEADERS += \
    const_data.h \
    mainwindow.h \
    positions_truct.h \
    board.h \
    search.h \
    square.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	resource/res.qrc

DISTFILES += \
	README.md
