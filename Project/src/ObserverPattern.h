#pragma once
#include <cstdio>
#include <unordered_map>
#include <random>
#include "Test.h"

#include "imgui.h"

std::random_device Device;
std::default_random_engine RandomEngine(Device());

namespace ObserverPattern
{
    template<class T>
    class Event
    {
    public:
        void Bind(T* obj, void(T::*fun)(void));
        bool Execute();
        void UnBind()
        {
            m_Obj = nullptr;
            m_Fun = nullptr;
        }
    private:
        T* m_Obj = nullptr;
        void (T::*m_Fun)() = nullptr;
    };

    template <class T>
    void Event<T>::Bind(T* obj, void(T::*fun)(void))
    {
        m_Obj = obj;
        m_Fun = fun;
    }

    template <class T>
    bool Event<T>::Execute()
    {
        if(m_Obj)
        {
            (m_Obj->*m_Fun)();
            return true;
        }
        return false;
    }

    // You must inherit from this interface so the multicast event can work
    class IObserver
    {
    public:
        void FuncToBind()
        {
            printf("This is an event function!\n");
            printf("The value of the variable is %f \n", TestVariable);
            printf("The value of the pointer is %f \n", *TestPointer);
        }

        float TestVariable = 10.f;
        float* TestPointer = new float(10.f);
    };
    
    class MulticastEvent
    {
    public:
        void Bind(IObserver* obj, void(IObserver::*fun)())
        {
            assert(m_Observers.size() == m_Functions.size());
            m_Observers.emplace_back(obj);
            m_Functions.emplace_back(fun);
        }
        void Execute() const
        {
            assert(m_Observers.size() == m_Functions.size());
            for (size_t i = 0; i < m_Observers.size(); i++)
            {
                IObserver* obj = m_Observers[i];
                void(IObserver::*fun)() = m_Functions[i];

                if(obj && fun)
                    (obj->*fun)();
            }
        }

        void UnBind(IObserver* obj, void(IObserver::*fun)())
        {
            assert(m_Observers.size() == m_Functions.size());
            for (size_t i = 0; i < m_Observers.size(); i++)
            {
                const IObserver* _obj = m_Observers[i];
                void(IObserver::*_fun)() = m_Functions[i];
                if(_obj == obj &&
                    _fun == fun)
                {
                    m_Observers.erase(m_Observers.begin() + i);
                    m_Functions.erase(m_Functions.begin() + i);
                }
            }
        }
        void UnBindAll()
        {
            m_Observers.clear();
            m_Functions.clear();
        }
    private:
        // We can get multiple function bindings from the same object!
        std::vector<IObserver*> m_Observers;
        std::vector<void(IObserver::*)()> m_Functions;
    };
    
    class ObserverTest : public ITest
    {
    public:
        ~ObserverTest() override
        {
            m_Event.UnBind();
            delete m_Observer;
            m_MulticastEvent.UnBindAll();
            m_Observers.clear();
        }
        
        bool Init() override
        {
            m_Observer = new IObserver();
            return true;
        }

        void Run() override{}
        void RunImGui() override
        {
            ImGui::Begin("Observer test");
            ImGui::TextColored(m_Observer ? ImVec4{0.0f, 1.0f, 0.0f, 1.0f} : ImVec4{1.f, 0.f, 0.f, 1.f} ,m_Observer ? "Valid observer" : "Invalid observer");
            
            if(ImGui::Button("Bind function!"))
            {
                if(m_Observer)
                {
                    m_Event.Bind(m_Observer, &IObserver::FuncToBind);
                    
                }
            }
            if(ImGui::Button("Delete observer"))
            {
                delete m_Observer;
                m_Observer = nullptr;
                m_Event.UnBind();
                printf("Observer has been deleted! \n");
            }
            if(ImGui::Button("Create a new observer"))
            {
                if(!m_Observer)
                {
                    m_Observer = new IObserver();
                    printf("A new observer has been created! \n");
                }
            }
            
            if(ImGui::Button("Invoke event!"))
            {
                if(m_Event.Execute())
                {
                    printf("Event invoked correctly! \n");
                }
                else
                {
                    printf("No object was bind to the event \n");
                }
            }
            ImGui::End();


            ImGui::Begin("Multicast observer test!");

            ImGui::Text("Observer count %i", m_Observers.size());
            
            if(ImGui::Button("Delete observer"))
            {
                if(!m_Observers.empty())
                {
                    IObserver* obj = m_Observers.back();
                    if(obj)
                    {
                        m_Observers.erase(m_Observers.end() - 1);
                        // Remember to unbind the function!
                        m_MulticastEvent.UnBind(obj, &IObserver::FuncToBind);
                        delete obj;
                    }
                }
            }
            if(ImGui::Button("Create a new observer"))
            {
                IObserver* obj = m_Observers.emplace_back(new IObserver());
                std::normal_distribution<float> uniform_dist(0.f, 100.f);
                obj->TestVariable = uniform_dist(RandomEngine);
                *(obj->TestPointer) = uniform_dist(RandomEngine);
                m_MulticastEvent.Bind(obj, &IObserver::FuncToBind);
                printf("A new observer has been created! \n");
            }
            
            if(ImGui::Button("Invoke event!"))
            {
                m_MulticastEvent.Execute();
                printf("Multicast event invoked!");
            }
            ImGui::End();   
        }

        void Shutdown() override
        {
            delete m_Observer;
            m_Observer = nullptr;
        }
    private:
        Event<IObserver> m_Event;
        IObserver* m_Observer = nullptr;

        std::vector<IObserver*> m_Observers;
        MulticastEvent m_MulticastEvent;
    };
    
}
