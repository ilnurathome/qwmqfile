#include "tfileconsumer.h"

#include <QTextStream>
#include <QDirIterator>
#include <QDebug>
#include <QDate>
#include <QTimer>

//QScriptValue fileConsumerToScriptValue(QScriptEngine *engine, FileConsumer* const &in)
//{ return engine->newQObject(in); }

//void fileConsumerFromScriptValue(const QScriptValue &object, FileConsumer* &out)
//{ out = qobject_cast<FileConsumer*>(object.toQObject()); }

QScriptValue tfileConsumerConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new TFileConsumer();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

QScriptValue tfileConsumerCommiterConstructor(QScriptContext *context, QScriptEngine *engine)
{
    QObject *object = new TFileConsumerCommiter();
    return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

bool TFileConsumer::initScriptEngine(QScriptEngine &engine)
{
    //    qScriptRegisterMetaType(&myEngine, fileConsumerToScriptValue, fileConsumerFromScriptValue);

    QScriptValue ctor = engine.newFunction(tfileConsumerConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&TFileConsumer::staticMetaObject, ctor);
    engine.globalObject().setProperty("TFileConsumer", metaObject);
    return true;
}

QString TFileConsumer::getPath() const
{
    return path;
}

void TFileConsumer::setPath(const QString &value)
{
    path = value;
}

void TFileConsumer::consume()
{
    if (path.size()>0)
        consume(path);
}


QString TFileConsumer::getArchPath() const
{
    return archPath;
}

void TFileConsumer::setArchPath(const QString &value)
{
    archPath = value;
}

QString TFileConsumer::getErrorPath() const
{
    return errorPath;
}

void TFileConsumer::setErrorPath(const QString &value)
{
    errorPath = value;
}

STD_FUNCTION<QString()> TFileConsumer::getArchPathFunc() const
{
    return archPathFunc;
}

QString TFileConsumer::callArchPathFuncGlobalScript()
{
    return archPathFuncScript.call(QScriptValue()).toString();
}

void TFileConsumer::setArchPathFunc(const STD_FUNCTION<QString ()> &value)
{
    archPathFunc = value;
}

void TFileConsumer::setArchPathFuncGlobal(const QString &value)
{
    qDebug() << __PRETTY_FUNCTION__;

    archPathFuncScript = myEngine->globalObject().property(value);
    //    archPathFunc = std::bind1st( std::mem_fun(&FileConsumer::callArchPathFuncGlobalScript), this);

    archPathFunc = boost::bind(&TFileConsumer::callArchPathFuncGlobalScript, this);

    qDebug() << __PRETTY_FUNCTION__<< ": " << archPathFuncScript.call(QScriptValue()).toString();
    qDebug() << __PRETTY_FUNCTION__<< ": " << archPathFunc();
}

QObject *TFileConsumer::getCommiter()
{
    return commiter;
}

void TFileConsumer::moveToThread(QObject *thread)
{
    QObject::moveToThread((QThread*)thread);
}

void TFileConsumer::quit()
{
    qDebug() << __PRETTY_FUNCTION__ <<":quit";
    isquit = true;
}


int TFileConsumer::getBatchSize() const
{
    return batchSize;
}

void TFileConsumer::setBatchSize(int value)
{
    batchSize = value;
}

TFileConsumer::TFileConsumer() : commiterThread(NULL), commiter(NULL), isquit(false)
{
    consuming = false;
    batchSize = 10;
    commiter = new TFileConsumerCommiter();
}

int TFileConsumer::init()
{
    commiterThread = new QThread();
    if (!commiterThread) {
        qCritical() << __PRETTY_FUNCTION__ << ": can't create commiter thread";
        return -1;
    }

    commiter->setArchPathFunc(archPathFunc);
    commiter->setArchPath(archPath);
    commiter->moveToThread(commiterThread);

    QObject::connect(commiter, SIGNAL(commited(TMessage<QFile>*)), this, SLOT(commit(TMessage<QFile>*)), Qt::QueuedConnection);
    QObject::connect(commiter, SIGNAL(rollbacked(TMessage<QFile>*)), this, SLOT(rollback(TMessage<QFile>*)), Qt::QueuedConnection);

    commiterThread->start();

    return 0;
}

TFileConsumer::~TFileConsumer()
{
    if (commiterThread) {
        commiterThread->quit();
        commiterThread->deleteLater();
    }

    if (commiter) {
        delete commiter;
    }
}

void TFileConsumer::consume(const QString &path)
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

    for (int i=0; i<batchSize && it.hasNext() && !isquit; i++) {
        QString filepath = it.next();
        QString filename = it.fileName();
        if (!valid(filename)) {
            //            qDebug() << filename << " is not valid - " << counter++;
            continue;
        }

        //                qDebug() << __PRETTY_FUNCTION__<< ":Consuming: " << filename << "\t" << filepath;

        //        QFile file(filepath);

        TMessage<QFile> *msg = new TMessage<QFile>();
        msg->getBody().setFileName(filepath);
        msg->setHeader("FileName", filename.toUtf8());
        msg->setHeader("FilePath", filepath.toUtf8());

        procceded++;
        emit message(msg);
    }
    processing = false;
    if (procceded == 0 || commited == procceded)
        consuming = false;

    //    qDebug() << __PRETTY_FUNCTION__<< ":Consumed : " << procceded;
}

void TFileConsumer::commit(TMessage<QFile> *msg)
{
    commited++;

    if (!processing) {
        //        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
            qDebug() << __PRETTY_FUNCTION__<< ":Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }

    if(msg) delete msg;
}

void TFileConsumer::rollback(TMessage<QFile> *msg)
{
    commited++;
    qDebug() << __PRETTY_FUNCTION__<< ":Rollback msg: " << msg->getHeaders().value("FileName");

    if (!processing) {
        //        qDebug() << "Wait for commit all : " << procceded << " : " << commited;
        if (commited == procceded) {
            qDebug() << __PRETTY_FUNCTION__<< ":Procced : " << procceded << " finish: " << commited;
            consuming = false;
        }
    }

    if(msg) delete msg;
}

bool TFileConsumer::valid(const QString &filename)
{
    if (filename == NULL) return false;
    if (filename.size() == 0) return false;
    if (filename.startsWith(".")) return false;
    //    if ((new QString("."))->compare(filename)) return false;
    //    if ((new QString(".."))->compare(filename)) return false;

    return true;
}


QString TFileConsumerCommiter::getArchPath() const
{
    return archPath;
}

void TFileConsumerCommiter::setArchPath(const QString &value)
{
    archPath = value;
}

bool TFileConsumerCommiter::initScriptEngine(QScriptEngine &engine)
{
    QScriptValue ctor = engine.newFunction(tfileConsumerCommiterConstructor);
    QScriptValue metaObject = engine.newQMetaObject(&TFileConsumerCommiter::staticMetaObject, ctor);
    engine.globalObject().setProperty("FileConsumerCommiter", metaObject);
    return true;
}

void TFileConsumerCommiter::commit(TMessage<QFile> *msg)
{
    //    qDebug() << __PRETTY_FUNCTION__<< ":" << msg.getHeaders().value("FileName") << " ; emiter: " << msg.getHeaders().value("emiter");
    QFile &file = msg->getBody();

    if (msg->getHeaders().contains("FileName")) {
        //            QString newDirPath = archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");
        QString newDirPath = archPathFunc.empty() ?
                    archPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm") :
                    archPath + "/" + archPathFunc();
        //            QString newDirPath = archPath + "/" + archPathFunc();
        QString filename = msg->getHeaders().value("FileName");

        //            qDebug() << __PRETTY_FUNCTION__<< ":Commiting: " << filename;

        QDir dir(newDirPath);
        if (!dir.exists()) {
            dir.mkpath(".");
            qDebug() << __PRETTY_FUNCTION__<< ":Create new dir : " << dir.absolutePath();
        }

        if (QFile::exists(newDirPath + "/" + filename)) {
            QFile::remove(newDirPath + "/" + filename);
        }

        if (!file.rename(newDirPath + "/" + filename)){
            qDebug() << __PRETTY_FUNCTION__<< ":Rename fail: " << newDirPath + "/" + filename << " : " << file.error() << " : " << file.errorString();
        }
        //            msg.setBody(NULL);
    } else {
        qDebug() << __PRETTY_FUNCTION__<< ":Msg body cast fail";
    }

    emit commited(msg);
}

void TFileConsumerCommiter::rollback(TMessage<QFile> *msg)
{
    emit rollbacked(msg);
}

void TFileConsumerCommiter::setArchPathFunc(const STD_FUNCTION<QString ()> &value)
{
    archPathFunc = value;
}
