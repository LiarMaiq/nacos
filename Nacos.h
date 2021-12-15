#pragma once
#include <list>
#include <map>
#include <future>
#include <string>
#include "MicroService.h"

struct ST_NACOS_BEAT
{
    bool enable;                    // beat or not
    int beatTime;                   // second
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
    ST_NACOS_BEAT beat;
    ST_NACOS_LIST list;
};

#ifdef _WIN32
class __declspec(dllexport) Nacos
#else
class Nacos
#endif
{
public:
    Nacos();
    ~Nacos();

public:
    int init();
    // size_t pushService(const std::string& service, std::list<nacos::Instance>& instances);
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    // print current services list
    void listServices();

protected:
    void run();
    void getInstances(const std::string service, std::vector<ST_MS_INSTANCE>& instances);

private:
    void* m_curlBeat;
    void* m_curlList;
    std::future<void> m_future;
    std::map<std::string, MicroService> m_microServices;
    bool m_stopping;
    std::function<void(int level, std::string log)> m_logger;
    ST_NACOS_CFG m_cfg;
    std::map<std::string, bool> m_addrs;
};