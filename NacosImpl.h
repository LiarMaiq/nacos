#pragma once
#include <vector>
#include <map>
#include <future>
#include <string>

class Service;

struct ST_NACOS_BEAT
{
    bool enable;                    // beat or not
    std::string path;               // uri
    std::map<std::string, std::string> queries;
};

struct ST_NACOS_LIST
{
    std::string path;               // uri
    std::map<std::string, std::string> queries;
};

struct ST_NACOS_CFG
{
    std::vector<std::string> addrs; // 10.29.195.12:8847
    int interval;                   // second
    ST_NACOS_BEAT beat;
    ST_NACOS_LIST list;
};

struct ST_INSTANCE
{
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

    bool operator==(const ST_INSTANCE& inst) const
    {
        return (inst.ip == this->ip) && (inst.port == this->port);
    }
    bool operator!=(const ST_INSTANCE& inst) const
    {
        return !(this->operator==(inst));
    }

    std::string handle()
    {
        return ip + ":" + std::to_string(port);
    }

    int weight_int()
    {
        return (int)(weight * 100);
    }

    bool available()
    {
        return healthy & enabled & valid & (port > 0) & (!ip.empty());
    }
};

class NacosImpl
{
public:
    NacosImpl();
    ~NacosImpl();

public:
    int init();
    // size_t pushService(const std::string& service, std::list<nacos::Instance>& instances);
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    // print current services list
    void listServices();

protected:
    void run();
    void getInstances(const std::string service, std::map<std::string, ST_INSTANCE>& instances);

private:
    void* m_curlBeat;
    void* m_curlList;
    std::future<void> m_future;
    // Service name -> Service*
    std::map<std::string, Service*> m_services;
    bool m_stopping;
    std::function<void(int level, std::string log)> m_logger;
    ST_NACOS_CFG m_cfg;
    std::map<std::string, bool> m_addrs;
};