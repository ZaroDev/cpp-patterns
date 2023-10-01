#pragma once

class ITest
{
public:
    virtual ~ITest() {}
    
    virtual bool Init() = 0;
    virtual void Run() = 0;
    virtual void RunImGui() = 0;
    virtual void Shutdown() = 0;
};
