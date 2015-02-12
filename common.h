#ifndef COMMON_H
#define COMMON_H

#include <QtScript>
#include <functional>
#include <QDebug>

// FIXME add check version of compiler and add std::function of C++11
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#define STD_FUNCTION boost::function

extern QScriptEngine* myEngine;

template <typename T>
class auto_free_cptr
{
    T *var;
    void (*f)(T*);
public:
    auto_free_cptr(T* val, void (*foo)(T*)) : var(val), f(foo) {
//        qDebug() << "auto_free_cptr::auto_free_cptr";
    }

    ~auto_free_cptr() {
//        qDebug() << "auto_free_cptr::~auto_free_cptr";
        if (var && f)
            f(var);
    }

    T* data() {
//        qDebug() << "auto_free_cptr::data()";
        return var;
    }

    operator T*() {
//        qDebug() << "auto_free_cptr::typecast";
        return var;
    }
};

#endif // COMMON_H
