#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>
#include <functional>
#include <iostream>
#include <stdexcept>

#include "collectionUtils.hpp"

template <typename... Types>
class EventHandler {
    std::function<void(Types...)> func = nullptr;

public:
    EventHandler() {
    }
    EventHandler(std::function<void(Types...)> func) {
        this->func = func;
    }

    virtual void operator()(Types... args) {
        Run(args...);
    }
    virtual void Run(Types... args) {
        try {
            func(args...);
        } catch(...) {
            std::cerr << "Event listener failed to run embedded function or embedded function was empty!" << std::endl;
        } 
    }
    virtual bool operator==(const EventHandler& other) {
        return this == &other;
    }
};

template <typename Host, typename... Types>
class ObjEventHandler : public EventHandler<Types...> {
public:
    Host* host = nullptr;
    std::function<void(Host*, Types...)> hostFunc;

    ObjEventHandler() {
    }
    ObjEventHandler(Host* host, std::function<void(Host*,Types...)> func) {
        this->host = host;
        this->hostFunc = func;
    }

    virtual void operator()(Types... args) {
        Run(args...);
    }
    virtual void Run(Types... args) {
        if (host == nullptr) {
            std::cerr << "Invoked a hosted event whose handler has a null host." << std::endl;
            throw std::runtime_error("Invoked a hosted event whose handler has a null host.");
        }
        hostFunc(host, args...);
    }
};

template <typename... Types>
class Event {
private:
    std::vector<EventHandler<Types...>*> listeners;
public:
    void AddListener(EventHandler<Types...>* listener) {
        listeners.push_back(listener);
    }
    void RemoveListener(EventHandler<Types...>* listener) {
        Remove(listeners, listener);
    }
    void Invoke(Types... parameters) {
        for (auto listener : listeners) {
            listener->Run(parameters...);
        }
    }
};

#endif