#pragma once

#include <string>
#include <chrono>

#include "Value.h"

class Profiler {
    public:
        Profiler(Profiler* parent, std::string* name);
        Profiler(std::string* name);
        Profiler(const char* name);

        Profiler* push(std::string* name);
        Profiler* push(const char* name);
        Profiler* pop();

        std::string* getName();

        std::vector<Profiler*>* getChildren();

        long getExecutions();

        float getSelfTime();
        float getChildTime();
        float getTotalTime();

        float getAvgSelfTime();
        float getAvgChildTime();
        float getAvgTotalTime();
    protected:
        Profiler* getChild(std::string* name);
        void begin();
        void end();
    private:
        Profiler* parent;
        std::string* name;
        std::vector<Profiler*>* children;

        long beginTimeNanos;
        long executions;
        float totalTime;
};

typedef std::vector<Profiler*> v_profiler;

extern Profiler* active_profiler;

#ifdef PROFILER_ENABLED
#define PROFILE_START(NAME)                         \
{                                                   \
  active_profiler = active_profiler->push(NAME);    \
}
#else
#define PROFILE_START(NAME) {}
#endif

#ifdef PROFILER_ENABLED
#define PROFILE_STOP()                    \
{                                          \
  active_profiler = active_profiler->pop();    \
}
#else
#define PROFILE_STOP() {}
#endif