#include "Service.h"

Service::Service()
{
}

Service::~Service()
{
}

const std::string Service::name()
{
    return m_name;
}

void Service::setName(const std::string name)
{
    m_name = name;
}

std::map<std::string, bool> Service::gets()
{
    std::map<std::string, bool> mss;
    std::unordered_map<std::string, ST_INSTANCE> insts = m_instances;
    for (auto& item : insts)
    {
        std::string addr = "http://" + item.second.handle();
        mss[addr] = item.second.available();
    }
    return mss;
}

std::string Service::get()
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
