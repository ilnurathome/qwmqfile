#ifndef INDEPONENTCONSUMER_H
#define INDEPONENTCONSUMER_H

#include <QHash>
#include <QObject>

template <typename T>
class IndeponentStorage
{
public:
    virtual void insert(const T &key) = 0;
    virtual bool contains(const T &key) = 0;
    virtual int remove (const T &key) = 0;
};


template <typename T>
class IndeponentMemoryStorage : public IndeponentStorage<T>
{
    QHash<T, int> objects;
public:
    IndeponentMemoryStorage() { qDebug() << __PRETTY_FUNCTION__; }
    ~IndeponentMemoryStorage() { qDebug() << __PRETTY_FUNCTION__; }

    virtual void insert(const T &key) {
        objects.insert(key, 0);
    }


    virtual bool contains(const T &key) {
        return objects.contains(key);
    }

    virtual int remove (const T &key) {
        return objects.remove(key);
    }
};

#endif // INDEPONENTCONSUMER_H
