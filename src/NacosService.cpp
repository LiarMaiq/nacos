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
 * ���·���ʵ���б�
 * @param instances ��ǰ�÷���ȫ������ʵ������˲����������е�ʵ���Ѳ�����
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

        // ������ʵ��Ȩ���������ʵ��������ֵ
        // ���ݲ�ֵ�����������ȱ�ٵ�ʵ��
        // ����ʵ��Ȩ�ر�С�����ڵ���Service::getʱ������ʵ���������
        int weight = item.second.weight_int() - m_instCount[item.first];
        for (int i = 0; i < weight; i++)
        {
            m_instQueue.push(item.first);
            m_instCount[item.first]++;
        }
    }
}
