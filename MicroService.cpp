#include "MicroService.h"

MicroService::MicroService()
{
    m_lastInst = 0;
}

MicroService::~MicroService()
{
}

const std::string MicroService::name()
{
    return m_name;
}

void MicroService::setName(const std::string name)
{
    m_name = name;
}

std::vector<std::pair<std::string, bool> > MicroService::gets()
{
    std::vector<std::pair<std::string, bool> > mss;
    std::vector<std::pair<ST_MS_INSTANCE, bool> > insts = m_instances;
    for (auto& item : insts)
    {
        std::string addr = "http://" + item.first.ip + ":" + std::to_string(item.first.port);
        mss.push_back({addr,item.second});
    }
    return mss;
}

/*
 * 简单的负载均衡
 */
std::string MicroService::get()
{
    ST_MS_INSTANCE inst;
    size_t s = m_instances.size();
    for (size_t i = 0; i < s; i++)
    {
        m_lastInst = m_lastInst%s;
        if (m_instances[m_lastInst].second)
        {
            inst = m_instances[m_lastInst].first;
            m_lastInst++;
            break;
        }
        m_lastInst++;
    }
    return "http://" + inst.ip + ":" + std::to_string(inst.port);
}

void MicroService::set(const ST_MS_INSTANCE& inst)
{
    bool available = (inst.healthy & inst.enabled & inst.valid);
	bool exist = false;
	for (size_t i = 0; i < m_instances.size(); i++)
	{
		if (m_instances[i].first == inst)
		{
			m_instances[i].second = available;
			exist = true;
		}
	}
	if (!exist)
	{
		m_instances.push_back({inst, available});
	}
}
