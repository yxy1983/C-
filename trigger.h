#ifndef _TRIGGER_H_
#define _TRIGGER_H_

#include <map>
#include <list>

class TriggerManager;
extern TriggerManager& GetManager();

template<int key>
class EventParam0;

#define DEFINE_TRIGGER_EVENT0(k)    \
template<>  \
class EventParam0<k> \
{\
public: \
    enum { ID = k, };           \
    EventParam0<k>(){} \
};

template<int key, typename T1, typename T2>
class EventParam2;

#define DEFINE_TRIGGER_EVENT2(k, T1, T2) \
template<>  \
class EventParam2<k, T1, T2> \
{   \
public: \
    enum { ID = k, };           \
    typedef typename T1 Type1;  \
    typedef typename T2 Type2;  \
    Type1 m_t1;                 \
    Type2 m_t2;                 \
    EventParam2<k, T1, T2>() : m_t1(), m_t2() {}  \
};

enum EVENT
{
    TRIGGER_EVENT_NONE = -1,

    TRIGGER_EVENT_V0 = 0,
    TRIGGER_EVENT_V2 = 2,
};

DEFINE_TRIGGER_EVENT0(TRIGGER_EVENT_V0);
DEFINE_TRIGGER_EVENT2(TRIGGER_EVENT_V2, int, int);

//////////////////////////////////////////////////////////////////////////

class IEventAction
{
public:
    virtual ~IEventAction() {}
    virtual void DoEvent() = 0;
protected:
    IEventAction(int k) : m_key(k) {}
    int m_key;
};


template<int key, typename T>
class EventAction0 : public IEventAction
{
public:
    typedef void (T::*Func)();

    EventAction0(T* t, Func f) : IEventAction(key), m_t(t), m_func(f) {}
    virtual ~EventAction0() {}

    virtual void DoEvent()
    {
        if (m_func && m_t)
            (m_t->*m_func)();
    }

protected:
    T*   m_t;
    Func m_func;
};

template<int key, typename T, typename T1, typename T2>
class EventAction2 : public IEventAction
{
public:
    typedef void (T::*Func)(const T1&, const T2&);

    EventAction2(T* t, Func f) : IEventAction(key), m_t(t), m_func(f) {}
    virtual ~EventAction2() {}

    virtual void DoEvent()
    {
        void * pParam = GetManager().GetEventParam(m_key);
        if (!pParam)
            return;

        EventParam2<key, T1, T2> * pReal = static_cast<EventParam2<key, T1, T2>* >(pParam);
        if (!pReal)
            return;
        
        if (m_t && m_func)
            (m_t->*m_func)(pReal->m_t1, pReal->m_t2);
    }

protected:
    T* m_t;
    Func m_func;
};

class TriggerManager
{
public:
    ~TriggerManager()
    {
        EventParamMap::iterator it = m_mapParam.begin();
        for (; it != m_mapParam.end(); ++it)
            delete it->second;
        m_mapParam.clear();

        EventActionMap::iterator itt = m_mapEvent.begin();
        for (; itt != m_mapEvent.end(); ++itt)
        {
            EventList& rList = itt->second;
            EventList::iterator ittt = rList.begin();
            for (; ittt != rList.end(); ++ittt)
            {
                IEventAction * p = *ittt;
                delete p;
            }
        }
        m_mapEvent.clear();
    }

    template<int key, typename T>
    void RegisterTriggerEvent(T * t, void (T::*pFun)())
    {
        EventAction0<key, T> * p = new EventAction0<key, T>(t, pFun);
        m_mapEvent[key].push_back(p);
    }

    template<int key, typename T, typename T1, typename T2>
    void RegisterTriggerEvent2(T * t, void (T::*pFun)(const T1&, const T2&))
    {
        EventAction2<key, T, T1, T2> * p = new EventAction2<key,  T, T1, T2>(t, pFun);
        m_mapEvent[key].push_back(p);

        EventParam2<key, T1, T2> * pp = new EventParam2<key, T1, T2>();
        m_mapParam[key] = (void*)pp;

    }

    void DoTriggerEvent_V0(int key)
    {
        EventList * pList = _GetEventList(key);
        if (!pList)
            return;

        EventList::iterator it = pList->begin();
        for (; it != pList->end(); ++it)
        {
            IEventAction * p = (*it);
            p->DoEvent();
        }
    }

    template<int key, typename T1, typename T2>
    void DoTriggerEvent_V2(const T1& t1, const T2& t2)
    {
        EventList * pList = _GetEventList(key);
        if (!pList)
            return;

        void * pEventParam = GetEventParam(key);
        if (!pEventParam)
            return;

        EventParam2<key, T1, T2> * pParam = static_cast<EventParam2<key, T1, T2>*>(pEventParam);
        if (!pParam)
            return;

        pParam->m_t1 = t1;
        pParam->m_t2 = t2;

        EventList::iterator it = pList->begin();
        for (; it != pList->end(); ++it)
        {
            IEventAction * p = (*it);
            p->DoEvent();
        }

    }

    void * GetEventParam(int key)
    {
        if (m_mapParam.find(key) != m_mapParam.end())
            return m_mapParam[key];

        return NULL;
    }

    TriggerManager() {};
protected:
    typedef std::list<IEventAction*> EventList;
    typedef std::map<int, EventList> EventActionMap;
    typedef std::map<int, void*> EventParamMap;

    EventList * _GetEventList(int  key)
    {
        if (m_mapEvent.find(key) == m_mapEvent.end())
            return NULL;

        return &(m_mapEvent[key]);
    }

    EventActionMap    m_mapEvent;
    EventParamMap       m_mapParam;
};

#endif

//////////////////////////////////////////////////////////////////////////
///main.cpp
//#include <iostream>
//#include "trigger.h"
//
//using namespace std;
//
//TriggerManager triggerManager;
//TriggerManager& GetManager()
//{
//    return triggerManager;
//}
//
//class TestA
//{
//public:
//    void DoTrigger()
//    {
//        std::cout << "do trigger" << endl;
//    }
//
//    void DoTrigger2(const int& a, const int& b)
//    {
//        std::cout << "do trigger 2 " << a << " " << b << endl;
//    }
//};
//
//int main()
//{
//    TestA testa;
//
//    triggerManager.RegisterTriggerEvent<TRIGGER_EVENT_V0>(&testa, &(TestA::DoTrigger));
//    triggerManager.RegisterTriggerEvent2<TRIGGER_EVENT_V2>(&testa, &(TestA::DoTrigger2));
//
//    triggerManager.DoTriggerEvent_V0(TRIGGER_EVENT_V0);
//    triggerManager.DoTriggerEvent_V2<TRIGGER_EVENT_V2, int, int>(10, 20);
//
//
//    system("pause");
//
//    return 0;
//}


