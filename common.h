#ifndef COMMON_H
#define COMMON_H

#include <QtScript>
#include <functional>

// FIXME add check version of compiler and add std::function of C++11
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#define STD_FUNCTION boost::function

extern QScriptEngine* myEngine;

#endif // COMMON_H
