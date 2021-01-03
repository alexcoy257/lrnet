#ifndef OSCSTREAMBUFFER_H
#define OSCSTREAMBUFFER_H

#include <QObject>
#include <cstring>
#include <QDebug>

class OSCStreamingBuffer: public QObject{
    constexpr static int inputSize = 1024;

    char _base[inputSize];
    char * _head = _base;
    size_t _remaining = inputSize;
    size_t mSize = 0;
public:
    OSCStreamingBuffer(QObject * parent=nullptr):QObject(parent){};
    void update(size_t bytesRead){
        _head += bytesRead;
        _remaining -= bytesRead;
        mSize = *((size_t*)_base);
        qDebug() << filled() << "bytes read. Expecting" <<mSize;
    }

    int32_t messageSize(){
        return mSize;
    }

    bool haveFullMessage(){
        return (_head - _base)+sizeof(size_t) >= (unsigned)mSize;
    }
    QByteArray * getMessage(){
        QByteArray * toRet = NULL;
        if(haveFullMessage()){
            toRet = new QByteArray(_base + sizeof(size_t), messageSize());
        }

        std::memmove(_base, _base + mSize+sizeof(size_t), filled()-mSize-sizeof(size_t));
        _remaining += mSize;
        _head = _base;
        mSize = *((size_t*)_base);
        return toRet;
    }
    void reset(){
        _head = _base;
        _remaining = inputSize;
    }
    char * head(){
        return _head;
    }
    char * base(){
        return _base;
    }
    size_t remaining(){
        return _remaining;
    }
    size_t filled(){
        return inputSize - _remaining;
    }
};

#endif // OSCSTREAMBUFFER_H
