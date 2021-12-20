#pragma once
#include <string>
#include <vector>
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
    std::map<std::string, ST_INSTANCE> m_instances;
    // queue<instance handle, status>
    ConcurrentQueue<std::pair<std::string, bool> >* m_instQueue;
    // instance handle -> current status
    std::map<std::string, bool> m_currentStatus;
};

