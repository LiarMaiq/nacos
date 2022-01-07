#include "NacosInstance.h"

NacosInstance::NacosInstance()
{
}

NacosInstance::~NacosInstance()
{
}

bool NacosInstance::operator==(const NacosInstance& inst) const
{
    return (inst.ip == this->ip) && (inst.port == this->port);
}
bool NacosInstance::operator!=(const NacosInstance& inst) const
{
    return !(this->operator==(inst));
}

std::string NacosInstance::handle()
{
    return ip + ":" + std::to_string(port);
}

int NacosInstance::weight_int()
{
    return (int)(weight * 100);
}

bool NacosInstance::available()
{
    return healthy & enabled & valid & (port > 0) & (!ip.empty());
}