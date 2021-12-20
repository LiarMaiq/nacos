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
        // ʵ��δʧЧ������uri���������
        if (inst.second == m_currentStatus[inst.first])
        {
            // ʵ�����òŷ���uri�����򷵻ؿմ�
            if (m_instances[inst.first].available())
                uri = "http://" + inst.first;
            m_instQueue->enqueue(inst);
            break;
        }
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
        ST_INSTANCE old = m_instances[item.first];
        m_instances[item.first] = item.second;

        int oldWeight = (int)(old.weight * 100);
        int weight = (int)(item.second.weight * 100);
        if (oldWeight != weight)
        {
            // ���ʵ��״̬��ʹ�����е�ʵ��ʧЧ��������������µ�ʵ��
            m_currentStatus[item.first] = !m_currentStatus[item.first];
            for (size_t i = 0; i < weight; i++)
                m_instQueue->enqueue({ item.first, m_currentStatus[item.first] });
        }
    }
}
