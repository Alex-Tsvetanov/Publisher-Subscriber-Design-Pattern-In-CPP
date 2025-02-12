#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <initializer_list>
#include <list>
#include <memory>

// === 1. Define the events and their traits ===

enum Events { A, B, C };

// === 2. The Publisher Base Class ===

// In our design the Publisher is templated on the enum type (here we use Events)
// and stores separate callback lists (here, vectors) for each event.
template <typename EventEnum, typename Publisher>
class BasePublisher;

template <typename ChildSubscriber, typename Publisher>
class Subscriber;

template <typename EventEnum, typename Publisher>
class BasePublisher {
    std::list<std::weak_ptr<Subscriber<Publisher>>> subscribers;
public:
    using EventsType = EventEnum;

    template<EventEnum event, typename... Ts>
    void emit(Ts... args);

    void subscribe(std::shared_ptr<Subscriber<Publisher>> sub) {
        subscribers.insert(sub, subscribers.end());
    }

    void unsubscribe(std::shared_ptr<Subscriber<Publisher>> sub) {
        for (auto it = subscribers.begin(); it != subscribers.end() ; )
        {
            const std::weak_ptr<Subscriber<Publisher>>& subscriber = *it;
            if (subscriber.expired())
            {
                it = subscribers.erase(it);
                continue;
            }
            if (std::shared_ptr<Subscriber<Publisher>> ptr = subscriber.lock())
            {
                if (ptr.get() == sub.get())
                {
                    it = subscribers.erase(it);
                    continue;
                }
            }
            it++;
        }
    }
};

class MyPublisher : public BasePublisher<Events, MyPublisher>
{
    template<Events event, typename... Ts>
    void emit(Ts... args);

    template<>
    void emit<Events::A>()
    {

    }

    template<>
    void emit<Events::B>(int a)
    {

    }

    template<>
    void emit<Events::C>(int a, std::string b)
    {

    }
};

template<typename T>
class function_signature;

template<typename C, typename Ret, typename... Args>
class function_signature<Ret(C::*)(Args...)> {
public:
    template<typename C2>
    using type = std::function<Ret(C2::*)(Args...)>;
};

template<typename ChildSubscriber, typename Publisher>
class Subscriber
{
protected:
    template<typename Publisher::EventsType event>
    void assign(typename function_signature<(&Publisher::emit<event>)>::type<ChildSubscriber> function)
    {

    }
public:

};


// MySubscriber inherits from Subscriber<MyPublisher> (recall that
// our Subscriber is defined for Publisher<Events>).
class MySubscriber : public Subscriber<MySubscriber, MyPublisher> {
public:
    // Here are the three event handlers.
    void handleA() {
        std::cout << "MySubscriber::handleA() called\n";
    }
    void handleB(int param1) {
        std::cout << "MySubscriber::handleB() called with param1 = " << param1 << "\n";
    }
    void handleC(int param1, std::string param2, std::vector<bool> param3) {
        std::cout << "MySubscriber::handleC() called with param1 = " << param1
            << ", param2 = " << param2
            << ", param3.size() = " << param3.size() << "\n";
    }

    // In the constructor we “assign” the event callbacks.
    MySubscriber() {
        // Note: We pass the address of the member function.
        this->assign<Events::A>(&MySubscriber::handleA);
        this->assign<Events::B>(&MySubscriber::handleB);
        this->assign<Events::C>(&MySubscriber::handleC);
    }
};


// === 5. Test the System in main() ===

int main() {
    MyPublisher pub;
    MySubscriber sub;

    // The subscriber registers itself with the publisher.
    pub.subscribe(sub);

    // Now the publisher can emit events.
    pub.emit<Events::A>();                 // calls sub.handleA()
    pub.emit<Events::B>(42);               // calls sub.handleB(42)
    pub.emit<Events::C>(7, "hello", std::vector<bool>{ true, false, true }); // calls sub.handleC(7, "hello", vector<bool>{...})

    return 0;
}
