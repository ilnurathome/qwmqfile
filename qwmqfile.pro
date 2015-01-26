#-------------------------------------------------
#
# Project created by QtCreator 2014-12-25T12:09:10
#
#-------------------------------------------------

QT       += core concurrent script

QT       -= gui

TARGET = qwmqfile
CONFIG   += console concurrent testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    fileconsumer.cpp \
    wmqproducer.cpp \
    exchange.cpp \
    message.cpp \
    filemessage.cpp \
    connectionpool.cpp \
    wmqconnection.cpp \
    qqueueproducer.cpp \
    qqueueconsumer.cpp \
    qqueueconsumerproducer.cpp \
    errormessage.cpp

HEADERS += \
    fileconsumer.h \
    wmqproducer.h \
    exchange.h \
    message.h \
    filemessage.h \
    connectionpool.h \
    iconnection.h \
    wmqconnection.h \
    qqueueproducer.h \
    qqueueconsumer.h \
    qqueueconsumerproducer.h \
    errormessage.h

win32:INCLUDEPATH += "C:/Program Files/IBM/WebSphere MQ/tools/cplus/include" \
"c:/Program Files/IBM/WebSphere MQ/tools/c/include"

win32:LIBS += "c:/Program Files/IBM/WebSphere MQ/tools/Lib/imqb23vn.Lib" \
"c:/Program Files/IBM/WebSphere MQ/tools/Lib/imqc23vn.Lib"

OTHER_FILES += \
    context.qs
