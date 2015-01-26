#include "fileconsumer.h"

#include <QTextStream>
#include <QDirIterator>
#include <QDebug>
#include <QDate>
#include <QTimer>
#include <QtConcurrent/QtConcurrentMap>



//Q_DECLARE_METATYPE(FileConsumer*)

//QScriptValue fileConsumerToScriptValue(QScriptEngine *engine, FileConsumer* const &in)
//{ return engine->newQObject(in); }

//void fileConsumerFromScriptValue(const QScriptValue &object, FileConsumer* &out)
//{ out = qobject_cast<FileConsumer*>(object.toQObject()); }

QScriptValue fileConsumerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new FileConsumer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}


bool FileConsumer::initScriptEngine(QScriptEngine &engine)
{
    //    qScriptRegisterMetaType(&myEngine, fileConsumerToScriptValue, fileConsumerFromScriptValue);

        QScriptValue ctor = engine.newFunction(fileConsumerConstructor);
        QScriptValue metaObject = engine.newQMetaObject(&FileConsumer::staticMetaObject, ctor);
        engine.globalObject().setProperty("FileConsumer", metaObject);
        return true;
}

QString FileConsumer::getPath() const
{
    return path;
}

void FileConsumer::setPath(const QString &value)
{
    path = value;
}

void FileConsumer::consume()
{
    if (path.size()>0)
        consume(path);
}


QString FileConsumer::getArchPath() const
{
    return archPath;
}

void FileConsumer::setArchPath(const QString &value)
{
    archPath = value;
}

QString FileConsumer::getErrorPath() const
{
    return errorPath;
}

void FileConsumer::setErrorPath(const QString &value)
{
    errorPath = value;
}

std::function<QString ()> FileConsumer::getArchPathFunc() const
{
    return archPathFunc;
}

void FileConsumer::setArchPathFunc(const std::function<QString ()> &value)
{
    archPathFunc = value;
}
FileConsumer::FileConsumer()
{
    consuming = false;
}

void FileConsumer::consume(const QString &path)
{
    if (consuming)
        return;

    consuming = true;
    procceded = 0;
    commited = 0;
    processing = true;
//    qDebug() << path;

    QDirIterator it(path);

//    long counter=0;

    while (it.hasNext()) {
        QString filepath = it.next();
        QString filename = it.fileName();
        if (!valid(filename)) {
//            qDebug() << filename << " is not valid - " << counter++;
            continue;
        }

        qDebug() << filename << "\t" << filepath;
        Message msg;
        msg.setHeader("FileName", filename);
        msg.setHeader("FilePath", filepath);


        msg.setBody((QObject *)new QFile(filepath));
        procceded++;
        emit message(msg);
    }
    processing = false;
    if (procceded == 0 || commited == procceded)
        consuming = false;

    qDebug() << "Consumed : " << procceded;
}

void FileConsumer::commit(Message msg)
{
    if (msg.getBody() != NULL) {
        QFile* file = qobject_cast<QFile*> (msg.getBody());

        if (file && msg.getHeaders().contains("FileName")) {
            QString newDirPath = archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
            QString filename = msg.getHeaders().value("FileName");

            QDir dir(newDirPath);
            if (!dir.exists()) {
                dir.mkpath(".");
                qDebug() << "Create new dir : " << dir.absolutePath();
            }
            if (!file->rename(newDirPath + "/" + filename)){
                qDebug() << "Rename fail : " << newDirPath + "/" + filename << " : " << file->error() << " : " << file->errorString();
            }
            delete file;
//            msg.setBody(NULL);
            commited++;
        } else {
            qDebug() << "Msg body cast fail";
        }
    } else {
        qDebug() << "Msg body is NULL";
    }

    if (!processing) {
//        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
                    qDebug() << "Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }
}

void FileConsumer::rollback(Message msg)
{
    commited++;
    qDebug() << "Rollback msg: " << msg.getHeaders().value("FileName");

    if (!processing) {
//        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
            qDebug() << "Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }
}

bool FileConsumer::valid(const QString &filename)
{
    if (filename == NULL) return false;
    if (filename.size() == 0) return false;
    if (filename.startsWith(".")) return false;
//    if ((new QString("."))->compare(filename)) return false;
//    if ((new QString(".."))->compare(filename)) return false;

    return true;
}


