#ifndef GLAEBHOERL_FRP_HPP
#define GLAEBHOERL_FRP_HPP

#include "FRP.h"

#include <list>

namespace glaebhoerl
{

template<typename... Ts>
struct Events<Ts...>::Impl
{
    std::list<std::function<void (Ts...)>> actions;
};

template<typename... Ts>
Events<Ts...>::Events(): m_impl()
{
}


template<typename... Ts>
Events<Ts...>::Events(const Generator& generator): m_impl(generator.m_impl)
{
}

template<typename... Ts>
Events<Ts...>::Events(const Events<Ts...>& other): m_impl(other.m_impl)
{
}

template<typename... Ts>
Events<Ts...> Events<Ts...>::mergedWith(const Events<Ts...>& other) const
{
    EventGen<Ts...> gen;
    this->react([gen] (Ts... args) { gen(args...); });
    other.react([gen] (Ts... args) { gen(args...); });
    return gen;
}

template<typename... Ts>
template<typename Function, typename ResultType>
Events<ResultType> Events<Ts...>::transformedBy(Function function) const
{
    EventGen<ResultType> gen;
    this->react([gen, function] (Ts... args) { gen(function(args...)); });
    return gen;
}

template<typename... Ts>
template<typename ResultType, typename, typename Object, typename>
Events<ResultType> Events<Ts...>::transformedBy(ResultType (Object::*method)() const) const
{
    return this->transformedBy([method] (Object obj) { return (obj.*method)(); });
}

template<typename... Ts>
template<typename Function, typename ResultType>
Events<ResultType> Events<Ts...>::transformedBy(const Variable<Function>& varFunction) const
{
    return this->withSnapshotAtFrontOf(varFunction).transformedBy([] (Function function, Ts... args) { return function(args...); });
}

template<typename... Ts>
template<typename Function, typename>
Events<Ts...> Events<Ts...>::filteredBy(Function function) const
{
    EventGen<Ts...> gen;
    this->react([gen, function] (Ts... args) {
        if (function(args...)) {
            gen(args...);
        }
    });
    return gen;
}

template<typename... Ts>
template<typename, typename Object, typename>
Events<Object> Events<Ts...>::filteredBy(bool (Object::*method)() const) const
{
    return this->filteredBy([method] (Object obj) { return (obj.*method)(); });
}

template<typename... Ts>
Events<Ts...> Events<Ts...>::filteredBy(const Variable<bool>& varCond) const
{
    return this->withSnapshotAtFrontOf(varCond).filteredBy([] (bool cond, Ts...) { return cond; }).template withoutPositions<0>();
}

template<typename... Ts>
template<typename Function, typename>
Events<Ts...> Events<Ts...>::filteredBy(const Variable<Function>& varFunction) const
{
    return this->withSnapshotAtFrontOf(varFunction).filteredBy([] (Function function, Ts... args) { return function(args...); }).template withoutPositions<0>();
}

template<typename... Ts>
template<typename Function, typename>
std::array<Events<Ts...>, 2> Events<Ts...>::partitionedBy(Function function) const
{
    return {
        this->filteredBy([function] (Ts... args) { return !function(args...); }),
        this->filteredBy(function)
    };
}

template<typename... Ts>
template<typename, typename Object, typename>
std::array<Events<Object>, 2> Events<Ts...>::partitionedBy(bool (Object::*method)() const) const
{
    return this->partitionedBy([method] (Object obj) { return (obj.*method)(); });
}

template<typename... Ts>
template<int n, typename Function, typename>
std::array<Events<Ts...>, n> Events<Ts...>::partitionedBy(Function function) const
{
    std::array<EventGen<Ts...>, n> generators;
    this->react([generators, function] (Ts... args) {
        int i = function(args...);
        if (i < 0 || n <= i) {
            abort();
        }
        generators[i].generate(args...);
    });
    std::array<Events<Ts...>, n> eventses;
    for (int i = 0; i < n; i++) {
        eventses[i] = generators[i];
    }
    return eventses;
}

template<typename... Ts>
template<typename... Us>
Events<Ts..., Us...> Events<Ts...>::taggedWith(Us... additionalArgs)
{
    typename Events<Ts..., Us...>::Generator gen; // see note [clang workaround]
    this->react([gen, additionalArgs...] (Ts... args) { gen(args..., additionalArgs...); });
    return gen;
}

template<typename... Ts>
template<typename... Us>
Events<Us..., Ts...> Events<Ts...>::taggedAtFrontWith(Us... additionalArgs)
{
    typename Events<Us..., Ts...>::Generator gen; // see note [clang workaround]
    this->react([gen, additionalArgs...] (Ts... args) { gen(additionalArgs..., args...); });
    return gen;
}


template<typename... Ts>
template<typename... Us>
Events<Ts..., Us...> Events<Ts...>::withSnapshotOf(const Variable<Us>&... vars) const
{
    typename Events<Ts..., Us...>::Generator gen; // see note [clang workaround]
    this->react([gen, vars...] (Ts... args) {
        gen(args..., vars.currentValue()...);
    });
    return gen;
}

template<typename... Ts>
template<typename... Us>
Events<Us..., Ts...> Events<Ts...>::withSnapshotAtFrontOf(const Variable<Us>&... vars) const
{
    typename Events<Us..., Ts...>::Generator gen; // see note [clang workaround]
    this->react([gen, vars...] (Ts... args) {
        gen(vars.currentValue()..., args...);
    });
    return gen;
}

// NOTE [clang workaround]
// We would like to write:
// EventGen<Ts..., Us...> gen;
// but this trips an assert in clang, so instead we write:
// typename Events<Ts..., Us...>::Generator gen;
// it could be worse.

template<typename... Ts>
template<int... ns>
WithPositions<Events<Ts...>, ns...> Events<Ts...>::positions() const
{
    typename WithPositions<Events<Ts...>, ns...>::Generator gen;
    this->react([gen] (Ts... args) {
        auto fwd = std::make_tuple(args...);
        gen(std::get<ns>(fwd)...);
    });
    return gen;
}

template<typename... Ts>
template<int... ns>
WithoutPositions<Events<Ts...>, ns...> Events<Ts...>::withoutPositions() const
{
    throw "Not implemented";
    // TODO
}

template<typename... Ts>
Events<> Events<Ts...>::stripped() const
{
    return positions<>();
}

template<typename... Ts>
Events<std::tuple<Ts...>> Events<Ts...>::tupled() const
{
    return this->transformedBy([] (Ts... args) { return std::make_tuple(args...); });
}

template<typename... Ts>
template<typename, typename Untupled>
Untupled Events<Ts...>::untupled() const
{
    throw "Not implemented";
   // TODO
}

namespace Impl
{
    template<typename... Ts>
    Events<Ts...> flattened(const Events<Events<Ts...>>& events)
    {
        EventGen<Ts...> gen;
        events.react([gen] (Events<Ts...> innerEvents) {
            innerEvents.react([gen] (Ts... args) {
                gen(args...);
            });
        });
        return gen;
    }
}

template<typename... Ts>
template<typename, typename InnerEventsType, typename>
InnerEventsType Events<Ts...>::flattened() const
{
    return glaebhoerl::Impl::flattened(*this);
}

template<typename... Ts>
Variable<int> Events<Ts...>::count() const
{
    return this->steppingFrom(0, [] (int n, Ts...) { return n + 1; });
}

template<typename... Ts>
template<typename, typename T>
Variable<T> Events<Ts...>::lastValueStartingWith(T initial) const
{
    return Variable<T>(initial, *this);
}

template<typename... Ts>
template<typename, typename T>
Variable<T> Events<Ts...>::lastValueStartingWithDefault() const
{
    return lastValueStartingWith(T());
}

template<typename... Ts>
template<int n, typename, typename T>
Variable<std::array<T, n>> Events<Ts...>::historyStartingWith(std::array<T, n> initial) const
{
    return this->steppingFrom(initial, [] (std::array<T, n> hist, T value) {
        for (int i = 0; i < n - 1; i++) {
            hist[i] = hist[i+1];
        }
        hist[n-1] = value;
        return hist;
    });
}

template<typename... Ts>
template<int n, typename, typename T>
Variable<std::array<T, n>> Events<Ts...>::historyStartingWithDefaults() const
{
    return historyStartingWith<n>(std::array<T, n>());
}

template<typename... Ts>
template<typename, typename T>
Variable<std::vector<T>> Events<Ts...>::history(int maxSize) const
{
    std::vector<T> initial;
    initial.reserve(maxSize);
    return this->steppingFrom(initial, [maxSize] (std::vector<T> hist, T value) {
        while (hist.size() > maxSize) {
            hist.erase(hist.begin());
        }
        hist.push_back(value);
        return hist;
    });
}

template<typename... Ts>
template<typename T, typename Function, typename>
Variable<T> Events<Ts...>::steppingFrom(T initial, Function function) const
{
    Mutable<T> mutVar(initial);
    this->react([function, mutVar] (Ts... args) {
        mutVar = function(mutVar.currentValue(), args...);
    });
    return mutVar.value();
}

template<typename... Ts>
template<typename T, typename, typename Function, typename>
Variable<T> Events<Ts...>::evolvingFrom(T initial) const
{
    Mutable<T> mutVar(initial);
    this->react([mutVar] (Function function) {
        mutVar.modify(function);
    });
    return mutVar.value();
}

template<typename... Ts>
template<typename, typename Action, typename>
void Events<Ts...>::exec() const
{
    this->react([] (Action action) {
        action();
    });
}

template<typename... Ts>
template<typename U, typename, typename T, typename ElementType>
Events<ElementType> Events<Ts...>::operator [](U key) const
{
    return this->transformedBy([key] (T arg) { return arg[key]; });
}

template<typename... Ts>
template<typename U, typename, typename T, typename ElementType>
Events<ElementType> Events<Ts...>::operator [](const Variable<U>& varKey) const
{
    return this->withSnapshotOf(varKey).transformedBy([] (T arg, U key) { return arg[key]; });
}

template<typename... Ts>
template<typename... Args, typename, typename Function, typename ResultType>
Events<ResultType> Events<Ts...>::operator ()(Args... args) const
{
    return this->transformedBy([args...] (Function function) { return function(args...); });
}

template<typename... Ts>
template<typename... Args, typename, typename Function, typename ResultType>
Events<ResultType> Events<Ts...>::operator ()(const Variable<Args>&... vars) const
{
    return this->withSnapshotOf(vars...).transformedBy([] (Function function, Args... args) { return function(args...); });
}

template<typename... Ts>
template<typename Action, typename>
void Events<Ts...>::react(Action action) const
{
    (void)reactUntilStopped(action);
}

template<typename... Ts>
template<typename Action, typename>
std::function<void ()> Events<Ts...>::reactUntilStopped(Action action) const
{
    if (auto impl = m_impl.lock()) {
        impl->actions.emplace_back(action);
        auto it = std::prev(impl->actions.end());
        auto _impl = m_impl;
        return [_impl, it] () mutable {
            if (auto impl = _impl.lock()) {
                impl->actions.erase(it);
                _impl.reset();
            }
        };
    } else {
        return [] { };
    }
}

template<typename... Ts>
void Events<Ts...>::operator =(Events<Ts...> other)
{
    m_impl = std::move(other.m_impl);
}

#define DEFINE_PREFIX_OPERATOR_FOR_EVENT(OP)                            \
    template<typename T, typename ResultType>                           \
    Events<ResultType> operator OP(const Events<T>& events)             \
    {                                                                   \
        return events.transformedBy([] (T value) { return OP value; }); \
    }                                                                   \

GLFRP_DO_FOR_EACH(DEFINE_PREFIX_OPERATOR_FOR_EVENT, +, -, !, ~, *)

#undef DEFINE_PREFIX_OPERATOR_FOR_EVENT

#define DEFINE_INFIX_OPERATORS_FOR_EVENT(OP)                                          \
    template<typename T, typename U, typename ResultType>                             \
    Events<ResultType> operator OP(const Events<T>& ea, U b)                          \
    {                                                                                 \
        return ea.transformedBy([b] (T a) { return a OP b; });                        \
    }                                                                                 \
                                                                                      \
    template<typename T, typename U, typename ResultType>                             \
    Events<ResultType> operator OP(const Events<T>& ea, const Variable<U>& vb)        \
    {                                                                                 \
        return ea.withSnapshotOf(vb).transformedBy([] (T a, U b) { return a OP b; }); \
    }                                                                                 \

GLFRP_DO_FOR_EACH(DEFINE_INFIX_OPERATORS_FOR_EVENT, +, -, *, /, %, ^, &, |, <<, >>, &&, ||, ==, !=, <, <=, >, >=)

#undef DEFINE_INFIX_OPERATORS_FOR_EVENT

namespace Impl
{
    template<typename... Ts>
    void forwardEvents(const Events<Ts...>& events, EventGen<Ts...> gen)
    {
        events.react([gen] (Ts... args) {
            gen(args...);
        });
    }
}

template<typename Iterator, typename EventsType, typename, typename>
EventsType mergedEvents(Iterator begin, Iterator end)
{
    typename EventsType::Generator gen;
    for (Iterator it = begin; it != end; ++it) {
        Impl::forwardEvents(*it, gen);
    }
    return gen;
}

template<typename Container, typename EventsType>
EventsType mergedEvents(const Container& container)
{
    return mergedEvents(container.cbegin(), container.cend());
}




template<typename... Ts>
Events<Ts...>::Generator::Generator(): m_impl(new Impl)
{
}

template<typename... Ts>
Events<Ts...>::Generator::Generator(const Generator& other): m_impl(other.m_impl)
{
}

template<typename... Ts>
void Events<Ts...>::Generator::generate(Ts... args) const
{
    for (const auto& action: m_impl->actions) {
        action(args...);
    }
}

template<typename... Ts>
void Events<Ts...>::Generator::operator ()(Ts... args) const
{
    generate(args...);
}

template<typename... Ts>
void Events<Ts...>::Generator::operator =(Generator other)
{
    m_impl = std::move(other.m_impl);
}




namespace Impl
{
    template<typename T, typename Requirement = glaebhoerl::Equals<decltype(GLFRP_VAL(T) == GLFRP_VAL(T)), bool>>
    Events<T> maybeFiltered(const Events<T>& changes, const std::shared_ptr<T>& value)
    {
        return changes.filteredBy([value] (T newValue) { return !(*value == newValue); });
    }

    template<typename T, typename... Args>
    Events<T> maybeFiltered(Events<T> changes, const Args&...)
    {
        return changes;
    }

    // XXX why doesn't the GLFRP_VAL way work?
    template<typename T/*, typename Requirement = decltype(GLFRP_VAL(T&) = GLFRP_VAL(T))*/>
    auto assignOrReconstruct(T& target, T source, int) -> decltype(target = source)
    {
        return target = source;
    }

    template<typename T>
    void assignOrReconstruct(T& target, T source, ...)
    {
        target.~T();
        new (&target) T(source);
    }
}


template<typename T>
Variable<T>::Variable(): Variable(T())
{
}

template<typename T>
Variable<T>::Variable(T value): m_value(new T(value)), m_changes()
{
}

template<typename T>
Variable<T>::Variable(T initial, const Events<T>& changes)
    : m_value(new T(initial)),
      m_changes(Impl::maybeFiltered(changes, m_value))
{
    auto value = m_value;
    m_changes.react([value] (T newValue) {
        Impl::assignOrReconstruct(*value, newValue, 666);
    });
}

template<typename T>
Variable<T>::Variable(const Variable<T>& other): m_value(other.m_value), m_changes(other.m_changes)
{
}

template<typename T>
template<typename Function, typename ResultType>
Variable<ResultType> Variable<T>::transformedBy(Function function) const
{
    return glaebhoerl::transformedBy(function, *this);
}

template<typename T>
template<typename ResultType, typename Object, typename>
Variable<ResultType> Variable<T>::transformedBy(ResultType (Object::*method)() const) const
{
    return this->transformedBy([method] (T obj) { return (obj.*method)(); });
}

template<typename T>
template<typename Function, typename ResultType>
Variable<ResultType> Variable<T>::transformedBy(const Variable<Function>& varFunction) const
{
    return varFunction(*this);
}

namespace Impl
{
    template<typename T>
    Variable<T> flattened(const Variable<Variable<T>>& varVar)
    {
        EventGen<T> gen;
        std::shared_ptr<std::function<void ()>> stop(new std::function<void ()>([] { }));
        varVar.always([gen, stop] (Variable<T> var) {
            (*stop)();
            *stop = var.alwaysUntilStopped([gen] (T value) {
                gen(value);
            });
        });
        return Variable<T>(varVar.currentValue().currentValue(), gen);
    }
}

template<typename T>
template<typename InnerVariableType, typename>
InnerVariableType Variable<T>::flattened() const
{
    return Impl::flattened(*this);
}

template<typename T>
template<typename U, typename ElementType>
Variable<ElementType> Variable<T>::operator [](U key) const
{
    return this->transformedBy([key] (T arg) { return arg[key]; });
}

template<typename T>
template<typename U, typename ElementType>
Variable<ElementType> Variable<T>::operator [](const Variable<U>& varKey) const
{
    return glaebhoerl::transformedBy([] (T val, U key) { return val[key]; }, *this, varKey);
}

template<typename T>
template<typename... Args, typename ResultType>
Variable<ResultType> Variable<T>::operator ()(Args... args) const
{
    return this->transformedBy([args...] (T function) { return function(args...); });
}

template<typename T>
template<typename... Args, typename ResultType>
Variable<ResultType> Variable<T>::operator ()(const Variable<Args>&... vars) const
{
    return glaebhoerl::transformedBy([] (T function, Args... args) { return function(args...); }, *this, vars...);
}

template<typename T>
template<typename Action, typename>
void Variable<T>::exec() const
{
    this->always([] (Action action) {
        action();
    });
}

template<typename T>
Events<T> Variable<T>::changes() const
{
    return m_changes;
}

template<typename T>
T Variable<T>::currentValue() const
{
    return *m_value;
}

template<typename T>
template<typename Action, typename>
void Variable<T>::always(Action action) const
{
    return (void)alwaysUntilStopped(action);
}

template<typename T>
template<typename Action, typename>
std::function<void ()> Variable<T>::alwaysUntilStopped(Action action) const
{
    action(currentValue());
    return changes().reactUntilStopped(action);
}

template<typename T>
void Variable<T>::operator =(Variable<T> other)
{
    m_changes = std::move(other.m_changes);
    m_value   = std::move(other.m_value);
}

#define DEFINE_PREFIX_OPERATOR_FOR_VARIABLE(OP)                       \
    template<typename T, typename ResultType>                         \
    Variable<ResultType> operator OP(const Variable<T>& var)          \
    {                                                                 \
        return var.transformedBy([] (T value) { return OP value; });  \
    }                                                                 \

GLFRP_DO_FOR_EACH(DEFINE_PREFIX_OPERATOR_FOR_VARIABLE, +, -, !, ~, *)

#undef DEFINE_PREFIX_OPERATOR_FOR_VARIABLE

#define DEFINE_INFIX_OPERATORS_FOR_VARIABLE(OP)                                    \
    template<typename T, typename U, typename ResultType>                          \
    Variable<ResultType> operator OP(const Variable<T>& va, U b)                   \
    {                                                                              \
        return va.transformedBy([b] (T a) { return a OP b; });                     \
    }                                                                              \
                                                                                   \
    template<typename T, typename U, typename ResultType>                          \
    Variable<ResultType> operator OP(T a, const Variable<U>& vb)                   \
    {                                                                              \
        return vb.transformedBy([a] (U b) { return a OP b; });                     \
    }                                                                              \
                                                                                   \
    template<typename T, typename U, typename ResultType>                          \
    Variable<ResultType> operator OP(const Variable<T>& va, const Variable<U>& vb) \
    {                                                                              \
        return transformedBy([] (T a, U b) { return a OP b; }, va, vb);            \
    }                                                                              \

GLFRP_DO_FOR_EACH(DEFINE_INFIX_OPERATORS_FOR_VARIABLE, +, -, *, /, %, ^, &, |, <<, >>, &&, ||, ==, !=, <, <=, >, >=)

#undef DEFINE_INFIX_OPERATORS_FOR_VARIABLE


template<typename T>
Variable<T> constant(T value)
{
    return Variable<T>(value);
}

template<typename Function, typename ResultType>
Variable<ResultType> sample(Function function, Events<> events)
{
    EventGen<ResultType> update;
    events.react([update, function] { update(function()); });
    return Variable<ResultType>(function(), update);
}

template<typename Function, typename... Ts, typename ResultType>
Variable<ResultType> transformedBy(Function function, const Variable<Ts>&... vars)
{
    std::array<Events<>, sizeof...(vars)> eventses = { vars.changes().stripped()... };
    std::function<ResultType ()> value = [function, vars...] { return function(vars.currentValue()...); };
    return sample(value, mergedEvents(eventses));
}

template<typename... Ts>
Variable<std::tuple<Ts...>> tupled(const Variable<Ts>&... vars)
{
    return transformedBy([] (Ts... args) { return std::make_tuple(args...); }, vars...);
}

template<typename... Ts>
Events<Ts...> variableEvents(const Variable<Events<Ts...>>& varEvents)
{
    EventGen<Ts...> gen;
    std::shared_ptr<std::function<void ()>> stop(new std::function<void ()>([] { }));
    varEvents.always([gen, stop] (Events<Ts...> events) {
        (*stop)();
        *stop = events.reactUntilStopped([gen] (Ts... args) {
            gen(args...);
        });
    });
    return gen;
}




template<typename T>
Mutable<T>::Mutable(): Mutable(T())
{
}

template<typename T>
Mutable<T>::Mutable(T value): m_update(), m_value(value, m_update)
{
}

template<typename T>
Variable<T> Mutable<T>::value() const
{
    return m_value;
}

template<typename T>
Mutable<T>::operator Variable<T>() const
{
    return m_value;
}

template<typename T>
T Mutable<T>::currentValue() const
{
    return m_value.currentValue();
}

template<typename T>
Events<T> Mutable<T>::changes() const
{
    return m_value.changes();
}

template<typename T>
void Mutable<T>::operator =(T value) const
{
    m_update(value);
}

template<typename T>
template<typename Action, typename>
void Mutable<T>::mutate(const Action& action) const
{
    T value = currentValue();
    action(value);
    m_update(value);
}

template<typename T>
template<typename Object, typename>
void Mutable<T>::mutate(void (Object::*method)()) const
{
    this->mutate([method] (Object& obj) { (obj.*method)(); });
}

template<typename T>
template<typename Function, typename>
void Mutable<T>::modify(const Function& function) const
{
    m_update(function(currentValue()));
}

template<typename T>
template<typename Object, typename>
void Mutable<T>::modify(Object (Object::*method)() const) const
{
    this->modify([method] (Object obj) { return (obj.*method)(); });
}

template<typename T>
template<typename, typename>
void Mutable<T>::operator ++() const
{
    this->mutate([] (T& value) { ++value; });
}

template<typename T>
template<typename, typename>
void Mutable<T>::operator --() const
{
    this->mutate([] (T& value) { --value; });
}

template<typename T>
template<typename, typename>
void Mutable<T>::operator ++(int) const
{
    this->mutate([] (T& value) { value++; });
}

template<typename T>
template<typename, typename>
void Mutable<T>::operator --(int) const
{
    this->mutate([] (T& value) { value--; });
}

#define DEFINE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE(OP)                         \
    template<typename T>                                                        \
    template<typename U, typename>                                              \
    void Mutable<T>::operator OP(U&& other) const                               \
    {                                                                           \
        this->mutate([&other] (T& value) { value OP std::forward<U>(other); }); \
    }                                                                           \

GLFRP_DO_FOR_EACH(DEFINE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=)

#undef DEFINE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE

template<typename T>
Mutable<T>::Mutable(const Mutable<T>& other): m_update(other.m_update), m_value(other.m_value)
{
}

template<typename T>
void Mutable<T>::operator =(Mutable<T> other)
{
    m_update = std::move(other.m_update);
    m_value  = std::move(other.m_value);
}

}

#endif
