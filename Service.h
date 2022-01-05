#pragma once
#include <unordered_map>
#include <vector>
#include <atomic>
#include <queue>
#include "nlohmann/json.hpp"
#include "NacosImpl.h"

class Service
{
public:
    Service();
    ~Service();

public:
    const std::string name();
	void setName(const std::string name);
    std::string get();
    void set(std::map<std::string, ST_INSTANCE>& instances);
    std::map<std::string, bool> gets();

private:
    std::string m_name;
    // instance handle -> instance
    std::unordered_map<std::string, ST_INSTANCE> m_instances;
    // instance handle -> instance count in queue
    std::unordered_map<std::string, std::atomic_int> m_instCount;
    //
    std::mutex m_mtx;
    std::queue<std::string> m_instQueue;
};

