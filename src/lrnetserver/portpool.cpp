#include "portpool.h"
#include <iostream>

/**
 * @brief PortPool::PortPool
 * @param low
 * @param high
 * Precondition: low < high.
 */
PortPool::PortPool(unsigned int low, unsigned int high):
    m_low(low),
    m_high(high),
    m_taken(new bool[m_high - m_low>0 ? m_high - m_low+1 : m_low - m_high+1]),
    //m_taken(new bool[50]),
    m_current(m_taken)
    {
    if (low > high){
        m_low = high;
        m_high = low;
    }

    for (bool * current = m_taken; current-m_taken <= m_high-m_low; current++)
        *current = false;


}

PortPool::~PortPool(){
    delete[] m_taken;
}

int PortPool::getPort(){
    if(!m_current)
        return -1;

    while (*m_current && m_current - m_taken < m_high - m_low){
        m_current += 1;
    }

    if (m_current - m_taken >= m_high - m_low){
        return -1;
    }

    *m_current = true;
    m_current += 1;
    int toRet = m_low + m_current - m_taken -1;
    return toRet;

}

void PortPool::returnPort(int port){
    if (port >= m_low && port <= m_high){
        m_current = m_taken + port - m_low;
        *m_current = false;
    }

}
