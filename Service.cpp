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

    for (auto& item : instances)
    {
        m_instances[item.first] = item.second;

        // ������ʵ��Ȩ���������ʵ��������ֵ
        // ���ݲ�ֵ�����������ȱ�ٵ�ʵ��
        // ����ʵ��Ȩ�ر�С�����ڵ���Service::getʱ������ʵ���������
        int weight = item.second.weight_int() - m_instCount[item.first];
        for (int i = 0; i < weight; i++)
        {
            bool result = m_instQueue->enqueue(*m_queueToken, item.first);
            m_instCount[item.first]++;
        }
    }
}
