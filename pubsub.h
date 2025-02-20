#pragma once
#include <functional>
#include <memory>
#include <list>
#include <unordered_map>
#include <concepts>
#include <type_traits>

namespace {
    template<size_t Id>
    struct counter {
        using tag = counter;

        struct generator {
            friend constexpr bool is_defined(tag) // The line that might cause a warning
            {
                return true;
            }
        };
        friend constexpr bool is_defined(tag);

        template<typename Tag = tag, bool = is_defined(Tag{}) >
        static constexpr bool exists(auto)
        {
            return true;
        }

        static constexpr bool exists(...)
        {
            return generator(), false;
        }
    };

    template<size_t Id = 0, typename = decltype([] {}) >
    constexpr size_t unique_id() {
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

    template <typename EventT>
    concept IsEvent = requires {
        typename EventT::func_t; // Check for func_t type alias
        { EventT::id } -> std::convertible_to<size_t>; // Check for id
    };

    template<auto Event> requires (IsEvent<decltype(Event)>)
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
        template<auto Event> requires (IsEvent<decltype(Event)>)
            void subscribe(std::function<typename decltype(Event)::func_t> f)
        {
            if (!events.contains(decltype(Event)::id))
                events[decltype(Event)::id] = new EventHandler<Event>();
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->subscribe(f);
        }

        template<auto Event, typename C, typename... Args> requires (IsEvent<decltype(Event)>)
            void subscribe(C* obj, void (C::* mem_fn)(Args...))
        {
            if (!events.contains(decltype(Event)::id))
                events[decltype(Event)::id] = new EventHandler<Event>;
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->subscribe(obj, mem_fn);
        }

        template<auto Event> requires (IsEvent<decltype(Event)>)
            void emit(auto... args)
        {
            reinterpret_cast<EventHandler<Event>*>(events[decltype(Event)::id])->emit(args...);
        }

        template<auto Event, typename C> requires (IsEvent<decltype(Event)>)
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
