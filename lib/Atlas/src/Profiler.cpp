#include "Profiler.h"

Profiler::Profiler(Profiler* parent, std::string* name) {
    this->parent = parent;
    this->name = name;
    this->executions = 0;
    this->totalTime = 0;
    this->children = new v_profiler(0);
}

Profiler::Profiler(std::string* name) {
    this->name = name;
    this->executions = 0;
    this->totalTime = 0;
    this->children = new v_profiler(0);
}

Profiler::Profiler(const char* name) {
    this->name = new std::string(name);
    this->executions = 0;
    this->totalTime = 0;
    this->children = new v_profiler(0);
}

std::string* Profiler::getName() {
    return this->name;
}

v_profiler* Profiler::getChildren() {
    return this->children;
}

Profiler* Profiler::getChild(std::string* name) {
    if (!children) {
        children = new v_profiler();
    }

    for (auto& child : *(this->children)) {
        if (*child->getName() == *name) {
            return child; 
        }
    }

    Profiler* child;
    child = new Profiler(this, name);
    this->children->push_back(child);
    return child;
}

Profiler* Profiler::push(std::string* name) {
    Profiler* child = getChild(name);;
    child->begin();
    return child;
}

Profiler* Profiler::push(const char* name) {
    return this->push(new std::string(name));
}

Profiler* Profiler::pop() {
    this->end();

    if (this->parent) {
        return this->parent;
    } else {
        return this;
    }
}

void Profiler::begin() {
    beginTimeNanos = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void Profiler::end() {
    auto endTimeNanos = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto duration = endTimeNanos - beginTimeNanos;
    this->totalTime += (duration / 1000000000.0);
    this->executions += 1;
}

long Profiler::getExecutions() {
    return this->executions;
}

double Profiler::getSelfTime() {
    return getTotalTime() - getChildTime();
}

double Profiler::getChildTime() {
    double time = 0.0;
    for (auto& child : *children) {
        time += child->getTotalTime();
    }
    return time;
}

double Profiler::getTotalTime() {
    return totalTime;
}

double Profiler::getAvgSelfTime() {
    return getSelfTime() / (double) getExecutions();
}

double Profiler::getAvgChildTime() {
    return getChildTime() / (double) getExecutions();
}

double Profiler::getAvgTotalTime() {
    return getTotalTime() / (double) getExecutions();
}

Profiler* active_profiler = new Profiler("main");