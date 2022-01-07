#pragma once
#include <unordered_map>
#include <queue>
#include <atomic>
#include <mutex>
#include "nlohmann/json.hpp"
#include "NacosInstance.h"

class NacosService
{
public:
    NacosService();
    ~NacosService();

public:
    const std::string name();
	void setName(const std::string name);
    std::string get();
    void set(std::map<std::string, NacosInstance>& instances);
    std::map<std::string, bool> gets();

private:
    std::string m_name;
    // instance handle -> instance
    std::unordered_map<std::string, NacosInstance> m_instances;
    // instance handle -> instance count in queue
    std::unordered_map<std::string, std::atomic_int> m_instCount;
    //
    std::mutex m_mtx;
    std::queue<std::string> m_instQueue;
};

