#pragma once
#include <list>
#include <map>
#include <future>
#include <string>
#include "MicroService.h"

struct ST_NACOS_CFG
{
    std::vector<std::string> addrs;
    int beatTime;
    std::string serviceName;
    std::string callback;
};

class Nacos
{
public:
    Nacos();
    ~Nacos();

public:
    int init(const ST_NACOS_CFG cfg);
    // size_t pushService(const std::string& service, std::list<nacos::Instance>& instances);
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    // print current services list
    void listServices();

protected:
    void run();
    void getInstances(const std::string service, std::vector<ST_MS_INSTANCE>& instances);

private:
    std::future<void> m_future;
    std::map<std::string, MicroService> m_microServices;
    bool m_stopping;
    std::function<void(int level, std::string log)> m_logger;
    ST_NACOS_CFG m_cfg;
};