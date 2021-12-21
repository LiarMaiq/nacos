#include "Service.h"

Service::Service()
{
    m_instQueue = new ConcurrentQueue<std::string>(1024);
    // One ProducerToken to ensure the order of item in the queue
    m_queueToken = new ProducerToken(*m_instQueue);
}

Service::~Service()
{
    delete m_queueToken;
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
    std::unordered_map<std::string, ST_INSTANCE> insts = m_instances;
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
    std::string inst;
    while (m_instQueue->try_dequeue(inst))
    {
        if (m_instances[inst].available())
        {
            uri = "http://" + inst;
            if (m_instances[inst].weight_int() >= m_instCount[inst])
            {
                bool result = m_instQueue->enqueue(*m_queueToken, inst);
                break;
            }
        }
        m_instCount[inst]--;
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
        m_instances[item.first] = item.second;

        // 计算新实例权重与队列中实例个数差值
        // 根据差值，补充队列中缺少的实例
        // 若新实例权重变小，则在调用Service::get时，出队实例不再入队
        int weight = item.second.weight_int() - m_instCount[item.first];
        for (int i = 0; i < weight; i++)
        {
            bool result = m_instQueue->enqueue(*m_queueToken, item.first);
            m_instCount[item.first]++;
        }
    }
}
