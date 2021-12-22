#include "NacosImpl.h"
#include "Service.h"
#if defined(_WIN32)
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "nlohmann/json.hpp"
#include "curl/curl.h"
#include <fstream>

NacosImpl::NacosImpl()
{
    m_stopping = false;
    m_curlBeat = 0;
    m_curlList = 0;
}

NacosImpl::~NacosImpl()
{
    m_stopping = true;
    m_future.wait();

    if (m_curlBeat)
        curl_easy_cleanup(m_curlBeat);
    if (m_curlList)
        curl_easy_cleanup(m_curlList);

    for (auto item : m_services)
        delete item.second;
}

int NacosImpl::init()
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_curlBeat = curl_easy_init();
    m_curlList = curl_easy_init();
    if (!m_curlBeat || !m_curlList)
    {
        if (m_logger)
            m_logger(40000, "Curl init failed");
        return -1;
    }

    nlohmann::json jCfg;
    std::fstream in("nacos.json");
    try
    {
        in >> jCfg;
    }
    catch (const std::exception& e)
    {
        if (m_logger)
            m_logger(40000, std::string("Parse file nacos.json exception:") + e.what());
        return -1;
    }
    if (jCfg.is_null())
    {
        if (m_logger)
            m_logger(40000, "Parse file nacos.json failed");
        return -1;
    }

    for (auto& addr : jCfg["addrs"])
        m_cfg.addrs.push_back(addr.get<std::string>());

    m_cfg.interval = jCfg["interval"];
    m_cfg.interval = m_cfg.interval < 1 ? 10 : m_cfg.interval;

    m_cfg.beat.enable = jCfg["beat"]["enable"];
    if (m_cfg.beat.enable)
    {
        m_cfg.beat.path = jCfg["beat"]["path"];
        m_cfg.beat.queries["serviceName"] = jCfg["beat"]["queries"]["serviceName"].get<std::string>();
        m_cfg.beat.queries["groupName"] = jCfg["beat"]["queries"]["groupName"].get<std::string>();
        m_cfg.beat.queries["ephemeral"] = jCfg["beat"]["queries"]["ephemeral"].get<bool>() ? "true" : "false";
        std::string beat = jCfg["beat"]["queries"]["beat"].dump();
        char* output = curl_escape(beat.c_str(), beat.length());
        m_cfg.beat.queries["beat"] = std::string(output);
        curl_free(output);
    }

    m_cfg.list.path = jCfg["list"]["path"];
    m_cfg.list.queries["groupName"] = jCfg["list"]["queries"]["groupName"].get<std::string>();
    m_cfg.list.queries["namespaceId"] = jCfg["list"]["queries"]["namespaceId"].get<std::string>();
    m_cfg.list.queries["clusters"] = jCfg["list"]["queries"]["clusters"].get<std::string>();

    m_future = std::async(std::launch::async, [this]()->void{this->run();});

    return 0;// In release no return will get "double free or corruption (fasttop)" error
}

void NacosImpl::setLogger(std::function<void(int level, std::string log)> logger)
{
    m_logger = logger;
}

void NacosImpl::listServices()
{
    for (auto& ms : m_services)
    {
        if (m_logger)
                m_logger(40000, ms.first + ":");
        std::vector<std::pair<std::string, bool> > mss = ms.second->gets();
        for (size_t i = 0; i < mss.size(); i++)
        {
            if (m_logger)
                m_logger(40000, "   " + mss[i].first + (mss[i].second ? "   true" : "  false"));
        }
    }
}

static size_t easy_read_cb(void* ptr, size_t size, size_t nmemb, void* userp)
{
    //struct input* i = userp;
    //size_t retcode = fread(ptr, size, nmemb, i->in);
    //i->bytes_read += retcode;
    //return retcode;
    return 0;
}

size_t easy_write_cb(void* data, size_t size, size_t count, void* userp) {
    const size_t n = size * count;
    if (userp)
    {
        std::string* body = (std::string*)userp;
        body->append((char*)data, n);
    }
    return n;
}

void NacosImpl::run()
{
    curl_easy_setopt(m_curlBeat, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(m_curlBeat, CURLOPT_READFUNCTION, easy_read_cb);
    curl_easy_setopt(m_curlBeat, CURLOPT_READDATA, 0);
    curl_easy_setopt(m_curlBeat, CURLOPT_WRITEFUNCTION, easy_write_cb);
    curl_easy_setopt(m_curlBeat, CURLOPT_WRITEDATA, 0);

    //curl_easy_setopt(e, CURLOPT_HEADERFUNCTION, easy_header_cb);
    //curl_easy_setopt(e, CURLOPT_HEADERDATA, (void*)ctx);
    curl_easy_setopt(m_curlList, CURLOPT_WRITEFUNCTION, easy_write_cb);

    while (!m_stopping)
    {
        // beat
        if (!m_cfg.beat.queries["serviceName"].empty() && m_cfg.beat.enable)
        {
            for (const std::string &item : m_cfg.addrs)
            {
                std::string u = "http://" + item + m_cfg.beat.path;
                u += "?serviceName=" + m_cfg.beat.queries["serviceName"];
                u += "&beat=" + m_cfg.beat.queries["beat"];
                u += "&ephemeral=" + m_cfg.beat.queries["ephemeral"];
                if (!m_cfg.beat.queries["groupName"].empty())
                    u += "&groupName=" + m_cfg.beat.queries["groupName"];

                curl_easy_setopt(m_curlBeat, CURLOPT_URL, u.c_str());
                std::string body;
                curl_easy_setopt(m_curlList, CURLOPT_WRITEDATA, (void*)&body);

                CURLcode res;
                int statusCodes = 0;
                res = curl_easy_perform(m_curlBeat);
                if (res == CURLcode::CURLE_OK)
                    curl_easy_getinfo(m_curlBeat, CURLINFO_RESPONSE_CODE, &statusCodes);
                if (statusCodes == 200)
                    m_addrs[item] = true;

                if (m_logger && statusCodes != 200)
                    m_logger(30000, "Nacos code:" + std::to_string(statusCodes) + " msg:" + body);
            }
        }

        // 
        for (auto& ms : m_services)
        {
            std::map<std::string, ST_INSTANCE> instances;
            getInstances(ms.first, instances);
            ms.second->set(instances);
        }
        
        //
#ifdef _WIN32
        Sleep(m_cfg.interval * 1000);
#else
        usleep(m_cfg.interval * 1000000);
#endif
    }
}

std::string NacosImpl::require(const std::string service)
{
    if (service.empty())
        return "";
    
    auto iter = m_services.find(service);
    if (iter == m_services.end())
    {
        std::map<std::string, ST_INSTANCE> instances;
        getInstances(service, instances);
        m_services[service] = new Service();
        m_services[service]->setName(service);
        m_services[service]->set(instances);
    }
    return m_services[service]->get();
}

/*
{
    "cacheMillis":3000,
    "checksum":"4ed1441a1950a11d913b4f62b4c2a525",
    "clusters":"",
    "dom":"approp-service",
    "env":"",
    "hosts":[{
            "clusterName":"DEFAULT",
            "enabled":true,
            "ephemeral":false,
            "healthy":false,
            "instanceId":"10.49.87.88#8002#DEFAULT#DEFAULT_GROUP@@approp-service",
            "ip":"10.49.87.88",
            "marked":false,
            "metadata":{
                "preserved.register.source":"SPRING_CLOUD"
            },
            "port":8002,
            "serviceName":"approp-service",
            "valid":false,
            "weight":1
        }],
    "lastRefTime":1627270340831,
    "metadata":{},
    "name":"DEFAULT_GROUP@@approp-service",
    "useSpecifiedURL":false
}
*/
void NacosImpl::getInstances(const std::string service, std::map<std::string, ST_INSTANCE>& instances)
{
    for (const auto &item : m_addrs)
    {
        // skip unavailable nacos addr
        if(!item.second)
            continue;

        std::string u = "http://" + item.first + m_cfg.list.path;
        u += "?serviceName=" + service;
        u += "&healthyOnly=true";
        if (!m_cfg.list.queries["groupName"].empty())
            u += "&groupName=" + m_cfg.list.queries["groupName"];
        if (!m_cfg.list.queries["namespaceId"].empty())
            u += "&namespaceId=" + m_cfg.list.queries["namespaceId"];
        if (!m_cfg.list.queries["clusters"].empty())
            u += "&clusters=" + m_cfg.list.queries["clusters"];

        curl_easy_setopt(m_curlList, CURLOPT_URL, u.c_str());
        std::string body;
        curl_easy_setopt(m_curlList, CURLOPT_WRITEDATA, (void*)&body);

        CURLcode res;
        int statusCodes = 0;
        res = curl_easy_perform(m_curlList);
        if (res != CURLcode::CURLE_OK)
            continue;

        curl_easy_getinfo(m_curlList, CURLINFO_RESPONSE_CODE, &statusCodes);
        if (statusCodes != 200)
            continue;

        ST_INSTANCE inst;
        if (body.empty())
            continue;

        nlohmann::json jBody = nlohmann::json::parse(body, nullptr, false, true);
        if (jBody.is_null())
            continue;

        nlohmann::json hosts = jBody["hosts"];
        if (!hosts.is_null() && hosts.is_array())
        {
            for (size_t i = 0; i < hosts.size(); i++)
            {
                nlohmann::json& host = hosts.at(i);
                if (host.find("ip") != host.end())
                    inst.ip = host["ip"].get<std::string>();

                if (host.find("port") != host.end())
                    inst.port = host["port"].get<int>();

                if (host.find("healthy") != host.end())
                    inst.healthy = host["healthy"].get<bool>();

                if (host.find("enabled") != host.end())
                    inst.enabled = host["enabled"].get<bool>();

                if (host.find("valid") != host.end())
                    inst.valid = host["valid"].get<bool>();

                if (host.find("marked") != host.end())
                    inst.marked = host["marked"].get<bool>();

                if (host.find("ephemeral") != host.end())
                    inst.ephemeral = host["ephemeral"].get<bool>();

                if (host.find("instanceId") != host.end())
                    inst.instanceId = host["instanceId"].get<std::string>();

                if (host.find("clusterName") != host.end())
                    inst.clusterName = host["clusterName"].get<std::string>();

                if (host.find("serviceName") != host.end())
                    inst.serviceName = host["serviceName"].get<std::string>();

                if (host.find("weight") != host.end())
                    inst.weight = host["weight"].get<double>();

                if (host.find("metadata") != host.end())
                    inst.metadata = host["metadata"].get<nlohmann::json>().dump();

                auto iter = instances.find(inst.handle());
                if (iter != instances.end())
                {
                    // update instance healthy = true
                    if (inst.healthy)
                        iter->second.healthy = inst.healthy;
                }
                else
                    instances[inst.handle()] = inst;
            }
        }
    }
}
