#include "Nacos.h"
#include "NacosImpl.h"

NacosImpl* _g_nacos_impl = 0;

Nacos::Nacos()
{
    _g_nacos_impl = new NacosImpl();
}

Nacos::~Nacos()
{
    if (_g_nacos_impl)
        delete _g_nacos_impl;
}

int Nacos::init()
{
    int rt = _g_nacos_impl->init();
    if (0 != rt)
    {
        delete _g_nacos_impl;
        _g_nacos_impl = 0;
    }
    return rt;
}

void Nacos::setLogger(std::function<void(int level, std::string log)> logger)
{
    if (_g_nacos_impl)
        _g_nacos_impl->setLogger(logger);
}

void Nacos::listServices()
{
    if (_g_nacos_impl)
        _g_nacos_impl->listServices();
}

std::string Nacos::require(const std::string service)
{
    std::string svc;
    if (_g_nacos_impl)
        svc = _g_nacos_impl->require(service);
    return svc;
}
