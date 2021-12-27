#ifndef OSCSTREAMBUFFER_H
#define OSCSTREAMBUFFER_H

#include <QObject>
#include <cstring>
#include <QDebug>

class OSCStreamingBuffer: public QObject{
    constexpr static int inputSize = 2048;

    char _base[inputSize];
    char * _head = _base;
    uint64_t _remaining = inputSize;
    uint64_t mSize = 0;
public:
    OSCStreamingBuffer(QObject * parent=nullptr):QObject(parent){};

    void update(uint64_t bytesRead){
        _head += bytesRead;
        _remaining -= bytesRead;
        memset(_head, 0xFF, qMin(_remaining, (uint64_t)sizeof(uint64_t)));
        mSize = *((uint64_t*)_base);
        //qDebug() << filled() << "bytes read. Expecting" <<mSize;
        //qDebug() << "Message now" << QByteArray(_base, filled() + qMin(_remaining, sizeof(uint64_t)));
    }

    int32_t messageSize(){
        return mSize;
    }

    bool haveFullMessage(){
        //qDebug() <<"have" <<(_head - _base)+sizeof(uint64_t) <<"need" <<mSize;
        return (_head - _base)+sizeof(uint64_t) >= (unsigned)mSize;
    }


    QByteArray * getMessage(){
        QByteArray * toRet = NULL;
        if(haveFullMessage()){
            toRet = new QByteArray(_base + sizeof(uint64_t), messageSize());

        //qDebug() <<"About to move this stuff" <<QByteArray(_base + mSize+sizeof(uint64_t), qMin(filled()-mSize-sizeof(uint64_t), 16UL));
        std::memmove(_base, _base + mSize+sizeof(uint64_t), filled()-mSize-sizeof(uint64_t));
        _remaining += mSize;
        _head -= mSize + sizeof(uint64_t);
        mSize = *((uint64_t*)_base);
        //qDebug() << "Updated message-Size" <<mSize <<QByteArray(_base, filled() + qMin(_remaining, sizeof(uint64_t)));
        //if (haveFullMessage())
        //    qDebug() <<"still have full message";
        return toRet;
        }
        return NULL;
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
    uint64_t remaining(){
        return _remaining;
    }
    uint64_t filled(){
        return inputSize - _remaining;
    }
};

#endif // OSCSTREAMBUFFER_H
