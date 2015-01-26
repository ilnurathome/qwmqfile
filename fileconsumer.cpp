#include "fileconsumer.h"
#include "filemessage.h"

#include <QTextStream>
#include <QDirIterator>
#include <QDebug>
#include <QDate>
#include <QTimer>

//QScriptValue fileConsumerToScriptValue(QScriptEngine *engine, FileConsumer* const &in)
//{ return engine->newQObject(in); }

//void fileConsumerFromScriptValue(const QScriptValue &object, FileConsumer* &out)
//{ out = qobject_cast<FileConsumer*>(object.toQObject()); }

QScriptValue fileConsumerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new FileConsumer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

QScriptValue fileConsumerCommiterConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new FileConsumerCommiter();
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

STD_FUNCTION<QString ()> FileConsumer::getArchPathFunc() const
{
    return archPathFunc;
}

QString FileConsumer::callArchPathFuncGlobalScript()
{
    return archPathFuncScript.call(QScriptValue()).toString();
}

void FileConsumer::setArchPathFunc(const STD_FUNCTION<QString ()> &value)
{
    archPathFunc = value;
}


void FileConsumer::setArchPathFuncGlobal(const QString &value)
{
    qDebug() << "setArchPathFuncGlobal";

    archPathFuncScript = myEngine->globalObject().property(value);
    //    archPathFunc = std::bind1st( std::mem_fun(&FileConsumer::callArchPathFuncGlobalScript), this);

    archPathFunc = boost::bind(&FileConsumer::callArchPathFuncGlobalScript, this);

    qDebug() << archPathFuncScript.call(QScriptValue()).toString();
    qDebug() << archPathFunc();
}

QObject *FileConsumer::getCommiter()
{
    return commiter;
}

void FileConsumer::moveToThread(QObject *thread)
{
    QObject::moveToThread((QThread*)thread);
}


int FileConsumer::getBatchSize() const
{
    return batchSize;
}

void FileConsumer::setBatchSize(int value)
{
    batchSize = value;
}

FileConsumer::FileConsumer() : commiterThread(NULL), commiter(NULL)
{
    consuming = false;
    batchSize = 10;
    commiter = new FileConsumerCommiter();
}

void FileConsumer::init()
{
    commiterThread = new QThread();
    commiter->setArchPathFunc(archPathFunc);
    commiter->setArchPath(archPath);
    commiter->moveToThread(commiterThread);

    QObject::connect(commiter, SIGNAL(commited(Message*)), this, SLOT(commit(Message*)), Qt::QueuedConnection);
    QObject::connect(commiter, SIGNAL(rollbacked(Message*)), this, SLOT(rollback(Message*)), Qt::QueuedConnection);

    commiterThread->start();
}

FileConsumer::~FileConsumer()
{
    if (commiterThread) {
        commiterThread->quit();
        commiterThread->deleteLater();
    }

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

    for (int i=0; i<batchSize && it.hasNext(); i++) {
        QString filepath = it.next();
        QString filename = it.fileName();
        if (!valid(filename)) {
            //            qDebug() << filename << " is not valid - " << counter++;
            continue;
        }

//                qDebug() << "Consuming: " << filename << "\t" << filepath;

        FileMessage *msg = new FileMessage();
        msg->setHeader("FileName", filename);
        msg->setHeader("FilePath", filepath);


        msg->setBody(new QFile(filepath));
        procceded++;
        emit message(msg);
    }
    processing = false;
    if (procceded == 0 || commited == procceded)
        consuming = false;

    qDebug() << "Consumed : " << procceded;
}

void FileConsumer::commit(Message *msg)
{
    commited++;

    if (!processing) {
        //        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
            qDebug() << "Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }
}

void FileConsumer::commitAndMove(Message *msg)
{
    if (msg->getBody() != NULL) {
        QFile* file = qobject_cast<QFile*> (msg->getBody());

        if (file && msg->getHeaders().contains("FileName")) {
            //            QString newDirPath = archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
            QString newDirPath = archPathFunc.empty() ?
                        archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") :
                        archPath + "/" + archPathFunc();
            //            QString newDirPath = archPath + "/" + archPathFunc();
            QString filename = msg->getHeaders().value("FileName");

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

void FileConsumer::rollback(Message *msg)
{
    commited++;
    qDebug() << "Rollback msg: " << msg->getHeaders().value("FileName");

    if (!processing) {
        //        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
            qDebug() << "Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }
}

void FileConsumer::rollbackAndMove(Message *msg)
{
    commited++;
    qDebug() << "Rollback msg: " << msg->getHeaders().value("FileName");

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


QString FileConsumerCommiter::getArchPath() const
{
    return archPath;
}

void FileConsumerCommiter::setArchPath(const QString &value)
{
    archPath = value;
}
bool FileConsumerCommiter::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(fileConsumerCommiterConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&FileConsumerCommiter::staticMetaObject, ctor);
    engine.globalObject().setProperty("FileConsumerCommiter", metaObject);
    return true;
}

void FileConsumerCommiter::commit(Message *msg)
{
//    qDebug() << "FileConsumerCommiter::commit : " << msg.getHeaders().value("FileName") << " ; emiter: " << msg.getHeaders().value("emiter");
    if (msg->getBody() != NULL) {
        QFile* file = qobject_cast<QFile*> (msg->getBody());

        if (file && msg->getHeaders().contains("FileName")) {
            //            QString newDirPath = archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
            QString newDirPath = archPathFunc.empty() ?
                        archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") :
                        archPath + "/" + archPathFunc();
            //            QString newDirPath = archPath + "/" + archPathFunc();
            QString filename = msg->getHeaders().value("FileName");

//            qDebug() << "Commiting: " << filename;

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
        } else {
            qDebug() << "Msg body cast fail";
        }
    } else {
        qDebug() << "Msg body is NULL";
    }

    emit commited(msg);
}

void FileConsumerCommiter::rollback(Message *msg)
{
    emit rollbacked(msg);
}

void FileConsumerCommiter::setArchPathFunc(const STD_FUNCTION<QString ()> &value)
{
    archPathFunc = value;
}
