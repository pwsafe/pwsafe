#include "./TimedTaskChain.h"

#ifndef __TESTING_TIMEDTASKCHAIN__
#include "../../core/PWSprefs.h"
#endif

// static
int TimedTaskChain::DefaultTaskDelay()
{
#ifndef __TESTING_TIMEDTASKCHAIN__
    static const int defaultDelay = PWSprefs::GetInstance()->GetPref(PWSprefs::TimedTaskChainDelay);
#else
    static const int defaultDelay = 100;
#endif
    return defaultDelay;
}

// static
TimedTaskChain& TimedTaskChain::CreateTaskChain(std::initializer_list<TaskType> tasks)
{
    return *new TimedTaskChain(tasks);
}

// static
TimedTaskChain& TimedTaskChain::CreateTaskChain(const TaskType &task)
{
    return CreateTaskChain({task});
}

//static
TimedTaskChain& TimedTaskChain::CreateTaskChain(std::initializer_list<TaskWithInterval> tasks)
{
    return *new TimedTaskChain(tasks);
}

TimedTaskChain::TimedTaskChain(std::initializer_list<TaskType> tasks):   m_errorHandler(nullptr)
{
    for (auto t: tasks)
        m_tasks.push_back({t, DefaultTaskDelay()});

    // even if m_tasks is empty
    m_nextTask = m_tasks.begin();

    // if m_tasks is empty, it would get destructed in the next timer callback
    Next();
}

TimedTaskChain::TimedTaskChain(std::initializer_list<TaskWithInterval> tasks): m_errorHandler{nullptr},
                                                                            m_tasks{tasks},
                                                                            m_nextTask{m_tasks.begin()}
{
    Next();
}

void TimedTaskChain::Next()
{
  Start(m_nextTask == m_tasks.end()? DefaultTaskDelay(): m_nextTask->second, wxTIMER_ONE_SHOT);
}

void TimedTaskChain::RunTask()
{
    if (m_nextTask != m_tasks.end()) {
        try {
            m_nextTask->first();
            m_nextTask++;
        }
        catch(std::exception& e) {
            if (m_errorHandler)
                m_errorHandler(e);

            m_nextTask = m_tasks.end();
        }
        Next();
    }
    else {
        delete this;
    }
}

void TimedTaskChain::Notify()
{
    RunTask();
}

#ifdef __TESTING_TIMEDTASKCHAIN__
#include <wx/app.h>
#include <iostream>

/*
 * Use this to compile from vim
 *
 *      set makeprg=g++\ -std=c++11\ %\ `wx-config\ --cxxflags`\ `wx-config\ --libs`
 *
 * And this to run it from command-line
 *
 *      g++ -D__TESTING_TIMEDTASKCHAIN__ TimedTaskChain.cpp -std=c++11 `wx-config --cxxflags --libs`
 *
 *
 */

class TaskApp: public wxApp
{
    const char *errmsg{"Here we throw!"};
    int total{0};
    wxStopWatch watch;

    void Quit()
    {
        static int testsRunning = TestsToRun.size();
        if (--testsRunning == 0)
            ExitMainLoop();
    }

    void TestSequentialExecution()
    {
    TimedTaskChain::CreateTaskChain( [this]() { total = 10; } )
                                .then( [this]() { assert(total == 10); total *= 2; })
                .then( [this]() { assert(total == 20); total += 3; })
                .then( [this]() { assert(total == 23); total += 34; })
                                .then( [this]() { assert(total == 57); Quit(); });
    }

    void TestErrorHandling()
    {
    TimedTaskChain::CreateTaskChain( [](void) {  } )
                                .then( []() {  })
                .then( [this]() { throw std::logic_error(this->errmsg); })
                .then( []() { assert(!!"Task executed after exception was thrown!" ); })
                                .OnError( [this](const std::exception& e) { assert(strcmp(e.what(), this->errmsg) == 0); Quit(); } );
    }

    void TestTaskDelays()
    {
        watch.Start();
        TimedTaskChain::CreateTaskChain( {{[this](){ watch.Pause(); assert(watch.Time() >= 100); watch.Resume();}, 100}} )
                                .then( [this](){ watch.Pause(); assert(watch.Time() >= 500); watch.Resume();}, 500 )
                                .then( [this](){ watch.Pause(); assert(watch.Time() >= 50);  watch.Resume();}, 50  )
                                .then( [this](){ watch.Pause(); assert(watch.Time() >= 300); Quit();}, 300 );
    }

    typedef std::mem_fun_t<void, TaskApp> TestFunction;
    std::array<TestFunction, 3> TestsToRun ={ { std::mem_fun(&TaskApp::TestSequentialExecution),
                                                std::mem_fun(&TaskApp::TestErrorHandling),
                                                std::mem_fun(&TaskApp::TestTaskDelays)
                                              }
    };

public:
  bool OnInit()
  {
        for ( auto t: TestsToRun )
            t(this);

    return true;
  }
};

wxIMPLEMENT_APP(TaskApp);

#endif //__TESTING_TIMEDTASKCHAIN__
