#pragma once
#include <functional>
#include <memory>
#include <list>
#include <unordered_map>
namespace {
    template<size_t Id>
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

    template<size_t Id = 0, typename = decltype([] {}) >
    consteval size_t unique_id() {
        if constexpr (not counter<Id>::exists(Id)) return Id;
        else return unique_id < Id + 1, decltype([] {}) > ();
    }
}


namespace pubsub {
    template<typename handler_t, size_t _id = unique_id < size_t{}, decltype([] {}) > () >
    struct Event
    {
        using func_t = handler_t;
        static constexpr size_t id = _id;

        constexpr Event() { }
    };

    template<auto Event>
    concept IsEvent = requires {
        typename decltype(Event)::func_t;
        //requires std::same_as<decltype(decltype(Event)::id), size_t>;
    };

    template<auto Event> requires (IsEvent<Event>)
        class EventHandler
    {
        using function_type = std::function<typename decltype(Event)::func_t>;
        std::list<function_type> callbacks;
        std::unordered_map<void*, typename std::list<function_type>::iterator> ptrs;
    public:
        EventHandler() {
        }

        void subscribe(const function_type& f)
        {
            callbacks.push_back(f);
        }

        template<typename C, typename... Args>
        void subscribe(C* obj, void (C::* mem_fn_ptr)(Args...))
        {
            function_type f = [obj, mem_fn_ptr](Args... args) noexcept -> void {
                ((*obj).*mem_fn_ptr)(args...);
                };
            ptrs[obj] = callbacks.insert(callbacks.end(), f);
        }

        template<typename C>
        void unsubscribe(C* obj)
        {
            if (ptrs.contains(obj))
                callbacks.erase(ptrs[obj]);
        }

        template<typename... Args>
        void emit(Args... args) {
            for (const auto& x : callbacks)
            {
                x(args...);
            }
        }
    };

    class Publisher
    {
        std::unordered_map<size_t, void*> events;
    public:
        Publisher() : events() {}
        ~Publisher()
        {
            for (auto& x : events)
            {
                free(x.second);
            }
        }
        template<auto Event> requires (IsEvent<Event>)
            void subscribe(std::function<typename decltype(Event)::func_t> f)
        {
            if (!events.contains(decltype(Event)::id))
                events[decltype(Event)::id] = new EventHandler<Event>();
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->subscribe(f);
        }

        template<auto Event, typename C, typename... Args> requires (IsEvent<Event>)
            void subscribe(C* obj, void (C::* mem_fn)(Args...))
        {
            if (!events.contains(decltype(Event)::id))
                events[decltype(Event)::id] = new EventHandler<Event>;
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->subscribe(obj, mem_fn);
        }

        template<auto Event> requires (IsEvent<Event>)
            void emit(auto... args)
        {
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->emit(args...);
        }

        template<auto Event, typename C> requires (IsEvent<Event>)
            void unsubscribe(C* obj)
        {
            if (!events.contains(decltype(Event)::id))
                return;
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->unsubscribe(obj);
        }

    };

    class Subscriber
    {
        std::list<Publisher*> publishers;
    public:
        Subscriber() : publishers() { }
        virtual ~Subscriber() { }

        virtual void subscribe_to(Publisher& p)
        {
            publishers.push_back(&p);
        }

        virtual void unsubscribe_from(Publisher& p) = 0;

        void unsubscribe_from_all() {
            for (auto& x : publishers)
            {
                unsubscribe_from(*x);
            }
        }
    };
}
