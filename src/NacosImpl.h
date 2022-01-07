#pragma once
#include <vector>
#include <mutex>
#include <future>
#include <unordered_map>
#include <deque>
#include "Nacos.h"
#include "NacosInstance.h"

class NacosService;

struct ST_NACOS_BEAT
{
    bool enable;                    // beat or not
    int interval;                   // second
    std::string path;               // uri
    std::map<std::string, std::string> queries;
};

struct ST_NACOS_LIST
{
    // ����ˢ����ȣ���ʾ�ж��ٸ�����ᶨ�ڴӷ������ȡ���µ�ʵ����Ϣ
    // ������������õ�˳������
    int refreshDepth;
    int interval;                   // second
    std::string path;               // uri
    std::map<std::string, std::string> queries;
};

struct ST_NACOS_LOGIN
{
    std::string path;               // uri
    std::string username;
    std::string password;
};

struct ST_NACOS_CFG
{
    std::vector<std::string> addrs; // 10.29.195.12:8847
    ST_NACOS_LOGIN login;
    ST_NACOS_BEAT beat;
    ST_NACOS_LIST list;
};

class Nacos::Impl
{
public:
    Impl();
    ~Impl();

public:
    int init();
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    std::map<std::string, std::map<std::string, bool>> listServices();

protected:
    void funcBeat();
    void funcList();
    void getInstances(const std::string service, std::map<std::string, NacosInstance>& instances);
    void login();
    void beat();
    void list();
    void pushLog(const int& level, const std::string& log);

private:
    void* m_curlBeat;
    void* m_curlList;
    void* m_curlLogin;
    std::future<void> m_futureBeat;
    std::future<void> m_futureList;
    // Service name -> Service*
    std::unordered_map<std::string, NacosService*> m_services;
    // The list of recent called service
    std::deque<std::string> m_recentServices;
    bool m_stopping;
    std::function<void(int level, std::string log)> m_logger;
    ST_NACOS_CFG m_cfg;
    std::map<std::string, bool> m_addrs;
    std::string m_token;
    std::mutex m_mtxCurlList;
    std::mutex m_mtxRequire;
};