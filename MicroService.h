#pragma once
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

struct ST_MS_INSTANCE
{
	std::string ip;
	int port;
    bool enabled;
	bool healthy;
	bool valid;
	bool ephemeral;
	bool marked;
    nlohmann::json metadata;
	std::string instanceId;
	std::string serviceName;
	std::string clusterName;
	double weight;

    bool operator==(const ST_MS_INSTANCE& inst) const
    {
        return (inst.ip == this->ip) && (inst.port== this->port);
    }
    bool operator!=(const ST_MS_INSTANCE& inst) const
    {
        return !(this->operator==(inst));
    }
};

class MicroService
{
public:
    MicroService();
    ~MicroService();

public:
    const std::string name();
	void setName(const std::string name);
    std::string get();
    void set(const ST_MS_INSTANCE& inst);
    std::vector<std::pair<std::string, bool> > gets();

private:
    std::string m_name;
    std::vector<std::pair<ST_MS_INSTANCE, bool> > m_instances;
    // load balancer
    size_t m_lastInst;
};

