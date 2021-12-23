#include "Nacos.h"
#include "NacosImpl.h"


Nacos::Nacos()
{
    m_impl = new Impl();
}

Nacos::~Nacos()
{
    if (m_impl)
        delete m_impl;
}

int Nacos::init()
{
    int rt = m_impl->init();
    if (0 != rt)
    {
        delete m_impl;
        m_impl = 0;
    }
    return rt;
}

void Nacos::setLogger(std::function<void(int level, std::string log)> logger)
{
    if (m_impl)
        m_impl->setLogger(logger);
}

std::map<std::string, std::map<std::string, bool>> Nacos::listServices()
{
    std::map<std::string, std::map<std::string, bool>> svcs;
    if (m_impl)
        svcs = m_impl->listServices();
    return svcs;
}

std::string Nacos::require(const std::string service)
{
    std::string svc;
    if (m_impl)
        svc = m_impl->require(service);
    return svc;
}
