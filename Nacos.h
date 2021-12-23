#pragma once
#include <map>
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
    std::string require(const std::string service);
    void setLogger(std::function<void(int level, std::string log)> logger);
    std::map<std::string, std::map<std::string, bool>> listServices();
};