#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <initializer_list>
#include <list>
#include <memory>
#include <unordered_map>

template<auto Id>
struct counter {
    using tag = counter;

    struct generator {
        friend consteval auto is_defined(tag)
        {
            return true;
        }
    };
    friend consteval auto is_defined(tag);

    template<typename Tag = tag, auto = is_defined(Tag{}) >
    static consteval auto exists(auto)
    {
        return true;
    }

    static consteval auto exists(...)
    {
        return generator(), false;
    }
};

template<auto Id = int{}, typename = decltype([] {}) >
consteval auto unique_id() {
    if constexpr (not counter<Id>::exists(Id)) return Id;
    else return unique_id<Id + 1>();
}

template<typename handler_t>
struct Event
{
    using func_t = handler_t;
    size_t id;

    constexpr Event() {
        id = unique_id();
    }
};

struct MyEvents
{
    static constexpr auto A = Event<void(*)()>();
    static constexpr auto B = Event<void(*)(int)>();
    static constexpr auto C = Event<void(*)(int, std::string, std::vector<bool>)>();
};

template<auto event>
void test()
{
    std::cout << event.id << " " << typeid(decltype(event)::func_t).name() << std::endl;
}

int main() {
    test<MyEvents::A>();
    test<MyEvents::B>();
    test<MyEvents::C>();
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
