#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QTextStream>

#define QSL(str) QStringLiteral(str)

//QTextStream qStdOut();

template <typename T>
void safeDelete(T* &p) {
    delete p;
    p = nullptr;
}


template <typename T>
void safeDeleteArray(T* &p) {
    delete[] p;
    p = nullptr;
}

#define ENTER() std::cout<<"Entering:"<<__FUNCTION__<<std::endl;

#endif // UTILS_H
