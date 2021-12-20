#include "Service.h"

Service::Service()
{
    m_instQueue = new ConcurrentQueue<std::pair<std::string, bool> >(1024);
}

Service::~Service()
{
    delete m_instQueue;
}

const std::string Service::name()
{
    return m_name;
}

void Service::setName(const std::string name)
{
    m_name = name;
}

std::vector<std::pair<std::string, bool> > Service::gets()
{
    std::vector<std::pair<std::string, bool> > mss;
    std::map<std::string, ST_INSTANCE> insts = m_instances;
    for (auto& item : insts)
    {
        std::string addr = "http://" + item.second.handle();
        mss.push_back({ addr,item.second.available() });
    }
    return mss;
}

std::string Service::get()
{
    std::string uri;
    std::pair<std::string, bool> inst;
    while (m_instQueue->try_dequeue(inst))
    {
        // 实例未失效，返回uri并重新入队
        if (inst.second == m_currentStatus[inst.first])
        {
            // 实例可用才返回uri，否则返回空串
            if (m_instances[inst.first].available())
                uri = "http://" + inst.first;
            m_instQueue->enqueue(inst);
            break;
        }
    }
    return uri;
}

/**
 * 更新服务实例列表
 * @param instances 当前该服务全部可用实例，因此不存在于其中的实例已不可用
 */
void Service::set(std::map<std::string, ST_INSTANCE>& instances)
{
    for (auto& item : m_instances)
    {
        auto iter = instances.find(item.first);
        if (iter == instances.end())
        {
            m_instances[item.first].healthy = false;
            continue;
        }
    }

    for (auto& item : instances)
    {
        ST_INSTANCE old = m_instances[item.first];
        m_instances[item.first] = item.second;

        int oldWeight = (int)(old.weight * 100);
        int weight = (int)(item.second.weight * 100);
        if (oldWeight != weight)
        {
            // 变更实例状态，使队列中的实例失效，并且重新入队新的实例
            m_currentStatus[item.first] = !m_currentStatus[item.first];
            for (size_t i = 0; i < weight; i++)
                m_instQueue->enqueue({ item.first, m_currentStatus[item.first] });
        }
    }
}
