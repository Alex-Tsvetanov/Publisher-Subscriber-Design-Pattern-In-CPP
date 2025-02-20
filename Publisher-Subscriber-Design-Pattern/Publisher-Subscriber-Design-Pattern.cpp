#include <iostream>
// Library
#include "pubsub.h"

// USAGE
using namespace pubsub;

struct MyEvents
{
    static constexpr auto A = Event<void()>();
    static constexpr auto B = Event<void(int)>();
    static constexpr auto C = Event<void(int, std::string, std::vector<bool>)>();
};

class MySubscriber : public Subscriber
{
public:
    MySubscriber() { }
    ~MySubscriber() {
        unsubscribe_from_all(); // required
    }
    void subscribe_to(Publisher& p)
    {
        p.subscribe<MyEvents::A>(this, &MySubscriber::handleEventA);
        p.subscribe<MyEvents::B>(this, &MySubscriber::handleEventB);
        p.subscribe<MyEvents::C>(this, &MySubscriber::handleEventC);
        Subscriber::subscribe_to(p); // required
    }
    void unsubscribe_from(Publisher& p)
    {
        p.unsubscribe<MyEvents::A>(this);
        p.unsubscribe<MyEvents::B>(this);
        p.unsubscribe<MyEvents::C>(this);
    }

    void handleEventA()
    {
        std::cout << "Subscriber is handling event A" << std::endl;
    }

    void handleEventB(int a)
    {
        std::cout << "Subscriber is handling event B for " << a << std::endl;
    }

    void handleEventC(int a, std::string b, std::vector<bool> c)
    {
        std::cout << "Subscriber is handling event C for " << a << " " << b << " " << c.size() << std::endl;
    }
};

int main() {
    //MySubscriber s; 
    {
        Publisher p;
        p.subscribe<MyEvents::A>([]() { std::cout << "Test lambda2" << std::endl; });
        p.subscribe<MyEvents::A>([]() { std::cout << "Test lambda3" << std::endl; });
        p.emit<MyEvents::A>();
        {
            MySubscriber s;
            s.subscribe_to(p);
            p.emit<MyEvents::A>();
            p.emit<MyEvents::B>(1);
        }
        p.emit<MyEvents::A>();
        p.emit<MyEvents::B>(1);
    }
    //MyPublisher pub;
    //std::shared_ptr<MySubscriber> sub = std::make_shared<MySubscriber>();
    //
    //// The subscriber registers itself with the publisher.
    //pub.subscribe(sub);
    //
    //// Now the publisher can emit events.
    //pub.emit<Events::A>();                 // calls sub.handleA()
    //pub.emit<Events::B>(42);               // calls sub.handleB(42)
    //pub.emit<Events::C>(7, std::string("hello"), std::vector<bool>{ true, false, true }); // calls sub.handleC(7, "hello", vector<bool>{...})

    return 0;
}
