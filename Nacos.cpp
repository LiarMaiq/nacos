#include "Nacos.h"
#include "cpprest/http_client.h"
#include "cpprest/http_msg.h"
#include "MicroService.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace utility;

Nacos::Nacos()
{
    m_stopping = false;
}

Nacos::~Nacos()
{
    m_stopping = true;
    m_future.wait();
}

int Nacos::init(const ST_NACOS_CFG cfg)
{
    m_cfg = cfg;
    // No beat, addr is available by default
    for (std::string addr : cfg.addrs)
        m_addrs[addr] = (!cfg.beat);
    m_future = std::async(std::launch::async, [this]()->void{this->run();});
    return 0;// In release no return will get "double free or corruption (fasttop)" error
}

void Nacos::setLogger(std::function<void(int level, std::string log)> logger)
{
    m_logger = logger;
}

void Nacos::listServices()
{
    for (auto& ms : m_microServices)
    {
        if (m_logger)
                m_logger(40000, ms.first + ":");
        std::vector<std::pair<std::string, bool> > mss = ms.second.gets();
        for (size_t i = 0; i < mss.size(); i++)
        {
            if (m_logger)
                m_logger(40000, "   " + mss[i].first + (mss[i].second ? "   true" : "  false"));
        }
    }
}

void Nacos::run()
{
    // Nacos
    //for (const std::string& item : nacoss)
    //{
    //    web::uri u = uri_builder(conversions::to_string_t("http://" + item + "/nacos/v1/ns/instance"))
    //        .append_query<string_t>(L"port", L"5561")
    //        .append_query<string_t>(L"healthy", L"true")
    //        .append_query<string_t>(L"ip", L"10.49.87.100")
    //        .append_query<string_t>(L"weight", L"1.0")
    //        .append_query<string_t>(L"serviceName", L"doge")
    //        .append_query<string_t>(L"encoding", L"GBK")
    //        .to_uri();
    //    http_client client(u);
    //    http_response rsp = client.request(methods::POST).wait();
    //}

    std::string callback = m_cfg.callback;
    std::string ip = callback.substr(0U, callback.find(':'));
    std::string httpPort = callback.substr(callback.find(':') + 1);
    std::string beat = "{ \"cluster\":\"\",\"ip\":\"" + ip + "\",\"metadata\":{},\"port\":" + httpPort + ",\"scheduled\":true,\"serviceName\":\"" + m_cfg.serviceName + "\",\"weight\":1}";
    web::http::client::http_client_config client_config;
    client_config.set_timeout(seconds(5));
    while (!m_stopping)
    {
        // beat
        if (!m_cfg.serviceName.empty() && !m_cfg.callback.empty() && m_cfg.beat)
        {
            for (const std::string &item : m_cfg.addrs)
            {
                web::uri u = uri_builder("http://" + item + "/nacos/v1/ns/instance/beat")
                                .append_query<string_t>("serviceName", m_cfg.serviceName)
                                .append_query<string_t>("beat", beat)
                                .to_uri();
                http_client client(u, client_config);
                try
                {
                    client.request(methods::PUT).then([this, item](http_response res){ m_addrs[item] = true; });
                }
                catch (std::exception const &e)
                {
                    if (m_logger)
                        m_logger(40000, "Nacos " + u.to_string() + " heartbeat exception:" + e.what());
                }
            }
        }

        // 
        for (auto& ms : m_microServices)
        {
            std::vector<ST_MS_INSTANCE> instances;
            getInstances(ms.first, instances);
            for (size_t i = 0; i < instances.size(); i++)
            {
                ms.second.set(instances[i]);
            }
        }
        
        //
        usleep(m_cfg.beatTime * 1000000);
    }
}

std::string Nacos::require(const std::string service)
{
    if (service.empty())
        return "";
    
    auto iter = m_microServices.find(service);
    if (iter == m_microServices.end())
    {
        std::vector<ST_MS_INSTANCE> instances;
        getInstances(service, instances);
        m_microServices[service] = MicroService();
        m_microServices[service].setName(service);
        for (size_t i = 0; i < instances.size(); i++)
        {
            m_microServices[service].set(instances[i]);
        }
    }
    return m_microServices[service].get();
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
void Nacos::getInstances(const std::string service, std::vector<ST_MS_INSTANCE>& instances)
{
    for (const auto &item : m_addrs)
    {
        // skip unavailable nacos addr
        if(!item.second)
            continue;

        web::http::client::http_client_config client_config;
        client_config.set_timeout(seconds(3));
        web::uri u = uri_builder("http://" + item.first + "/nacos/v1/ns/instance/list")
                        .append_query<string_t>("serviceName", service)
                        .to_uri();
        http_client client(u, client_config);
        try
        {
            json::value body = client.request(methods::GET).then([](http_response res) -> auto
                                              { return res.extract_json(); }).get();
            ST_MS_INSTANCE inst;
            if (!body.is_null())
            {
                json::value hosts = body["hosts"];
                if (!hosts.is_null() && hosts.is_array())
                {
                    for (size_t i = 0; i < hosts.size(); i++)
                    {
                        json::value &host = hosts.at(i);
                        if (host.has_string_field("ip"))
                            inst.ip = host["ip"].as_string();

                        if (host.has_integer_field("port"))
                            inst.port = host["port"].as_integer();

                        if (host.has_boolean_field("healthy"))
                            inst.healthy = host["healthy"].as_bool();

                        if (host.has_boolean_field("enabled"))
                            inst.enabled = host["enabled"].as_bool();

                        if (host.has_boolean_field("valid"))
                            inst.valid = host["valid"].as_bool();

                        if (host.has_boolean_field("marked"))
                            inst.marked = host["marked"].as_bool();

                        if (host.has_boolean_field("ephemeral"))
                            inst.ephemeral = host["ephemeral"].as_bool();

                        if (host.has_string_field("instanceId"))
                            inst.instanceId = host["instanceId"].as_string();

                        if (host.has_string_field("clusterName"))
                            inst.clusterName = host["clusterName"].as_string();

                        if (host.has_string_field("serviceName"))
                            inst.serviceName = host["serviceName"].as_string();

                        if (host.has_double_field("weight"))
                            inst.weight = host["weight"].as_double();

                        if (host.has_field("metadata"))
                            inst.metadata = host["metadata"];

                        bool ext = false;
                        for (size_t i = 0; i < instances.size(); i++)
                        {
                            if (instances[i] == inst)
                            {
                                ext = true;
                                // update instance healthy = true
                                if (inst.healthy)
                                    instances[i].healthy = inst.healthy;
                                break;
                            }
                        }
                        if (!ext)
                            instances.push_back(inst);
                    }
                }
            }
        }
        catch (std::exception const &e)
        {
            if (m_logger)
                m_logger(40000, "Nacos " + u.to_string() + " instance list: " + e.what());
        }
    }
}
