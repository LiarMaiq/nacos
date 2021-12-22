#pragma once
#include <string>
#include <functional>

#ifdef _WIN32
class __declspec(dllexport) Nacos
#else
class Nacos
#endif
{
public:
    Nacos();
    ~Nacos();

public:
    int init();
    // size_t pushService(const std::string& service, std::list<nacos::Instance>& instances);
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    // print current services list
    void listServices();
};