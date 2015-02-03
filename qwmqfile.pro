#-------------------------------------------------
#
# Project created by QtCreator 2014-12-25T12:09:10
#
#-------------------------------------------------

QT       += core concurrent script network

QT       -= gui

TARGET = qwmqfile
CONFIG   += console concurrent
CONFIG   -= app_bundle

#QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

SOURCES += main.cpp \
    fileconsumer.cpp \
    wmqproducer.cpp \
    exchange.cpp \
    message.cpp \
    filemessage.cpp \
    connectionpool.cpp \
    wmqconnection.cpp \
    errormessage.cpp \
    wmqconsumer.cpp \
    bytemessage.cpp \
    testmsgprocessor.cpp \
    fileproducer.cpp \
    file2bytearrayprocess.cpp \
    tfileconsumer.cpp \
    httpproducer.cpp

HEADERS += \
    fileconsumer.h \
    wmqproducer.h \
    exchange.h \
    message.h \
    filemessage.h \
    connectionpool.h \
    iconnection.h \
    wmqconnection.h \
    errormessage.h \
    common.h \
    wmqconsumer.h \
    bytemessage.h \
    testmsgprocessor.h \
    fileproducer.h \
    file2bytearrayprocess.h \
    tfileconsumer.h \
    httpproducer.h


win32:INCLUDEPATH += "C:/Program Files/IBM/WebSphere MQ/tools/cplus/include" \
"c:/Program Files/IBM/WebSphere MQ/tools/c/include" \
"c:/Programs/boost_1_54_0"

win32:LIBS += "c:/Program Files/IBM/WebSphere MQ/tools/Lib/imqb23vn.Lib" \
"c:/Program Files/IBM/WebSphere MQ/tools/Lib/imqc23vn.Lib"

unix:INCLUDEPATH += /opt/mqm/inc
unix:LIBS += /opt/mqm/lib64/libmqic_r.so \
/opt/mqm/lib64/libmqe_r.so \
/opt/mqm/lib64/libmqecs_r.so \
/opt/mqm/lib64/libmqm_r.so \
/opt/mqm/lib64/4.1/libimqb23gl_r.so \
/opt/mqm/lib64/4.1/libimqc23gl_r.so

OTHER_FILES += \
    context.qs
