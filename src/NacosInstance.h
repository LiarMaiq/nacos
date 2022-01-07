#pragma once
#include <string>

class NacosInstance
{
public:

    NacosInstance();
    ~NacosInstance();

    bool operator==(const NacosInstance& inst) const;
    bool operator!=(const NacosInstance& inst) const;
    std::string handle();
    int weight_int();
    bool available();

    std::string ip;
    int port;
    bool enabled;
    bool healthy;
    bool valid;
    bool ephemeral;
    bool marked;
    std::string metadata;   // json
    std::string instanceId;
    std::string serviceName;
    std::string clusterName;
    double weight;
};

