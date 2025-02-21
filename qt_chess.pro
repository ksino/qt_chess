QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    board.cpp \
    positions.cpp \
    search.cpp \
    square.cpp \
    zobrist.cpp

HEADERS += \
    const_data.h \
    mainwindow.h \
    board.h \
    positions.h \
    search.h \
    square.h \
    zobrist.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	resource/res.qrc

DISTFILES += \
	README.md
