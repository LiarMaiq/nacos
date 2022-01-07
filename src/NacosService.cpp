#include "NacosService.h"

NacosService::NacosService()
{
}

NacosService::~NacosService()
{
}

const std::string NacosService::name()
{
    return m_name;
}

void NacosService::setName(const std::string name)
{
    m_name = name;
}

std::map<std::string, bool> NacosService::gets()
{
    std::map<std::string, bool> mss;
    std::unordered_map<std::string, NacosInstance> insts = m_instances;
    for (auto& item : insts)
    {
        std::string addr = "http://" + item.second.handle();
        mss[addr] = item.second.available();
    }
    return mss;
}

std::string NacosService::get()
{
    std::unique_lock<std::mutex> lck(m_mtx);
    std::string uri;
    std::string inst;
    while (true)
    {
        if (m_instQueue.empty())
            break;

        inst = m_instQueue.front();
        m_instQueue.pop();
        if (m_instances[inst].available())
        {
            uri = "http://" + inst;
            if (m_instances[inst].weight_int() >= m_instCount[inst])
            {
                m_instQueue.push(inst);
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
void NacosService::set(std::map<std::string, NacosInstance>& instances)
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

    std::unique_lock<std::mutex> lck(m_mtx);
    for (auto& item : instances)
    {
        m_instances[item.first] = item.second;

        // 计算新实例权重与队列中实例个数差值
        // 根据差值，补充队列中缺少的实例
        // 若新实例权重变小，则在调用Service::get时，出队实例不再入队
        int weight = item.second.weight_int() - m_instCount[item.first];
        for (int i = 0; i < weight; i++)
        {
            m_instQueue.push(item.first);
            m_instCount[item.first]++;
        }
    }
}
