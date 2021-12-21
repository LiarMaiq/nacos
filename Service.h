#pragma once
#include <unordered_map>
#include <vector>
#include <atomic>
#include "nlohmann/json.hpp"
#include "concurrentqueue/concurrentqueue.h"
#include "Nacos.h"

using namespace moodycamel;

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
    std::vector<std::pair<std::string, bool> > gets();

private:
    std::string m_name;
    // instance handle -> instance
    std::unordered_map<std::string, ST_INSTANCE> m_instances;
    // queue<instance handle>
    ConcurrentQueue<std::string>* m_instQueue;
    ProducerToken* m_queueToken;
    // instance handle -> instance count in queue
    std::unordered_map<std::string, std::atomic_int> m_instCount;
};

