#ifndef PORTPOOL_H
#define PORTPOOL_H
#include <QVector>

class PortPool
{
    int m_low;
    int m_high;
    bool * m_taken;
    bool * m_current;
public:
    PortPool(unsigned int low = 61002, unsigned int high=61200);
    int getPort();
    void returnPort(int port);
    bool * getCurrentBoolAddress(){return m_taken;};
    bool * getCurrentCBoolAddress(){return m_current;};

    ~PortPool();
};

#endif // PORTPOOL_H
