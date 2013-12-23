#ifndef GLAEBHOERL_FRP_H
#define GLAEBHOERL_FRP_H

#include <array>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "FRP_util.h"

#define VAL(TYPE)    std::declval<TYPE>()
#define IS_LEGAL     decltype
#define TYPE_OF(...) WithoutQualifiers<decltype(__VA_ARGS__)>

namespace glaebhoerl
{

template<typename>
struct Variable;

template<typename... Ts>
struct Events
{
    struct Generator;

    Events();
    Events(const Generator&);
    Events(const Events<Ts...>&);

    Events<Ts...> mergedWith(const Events<Ts...>&) const;

    template<typename Function, typename ResultType = TYPE_OF( VAL(Function)(VAL(Ts)...) )>
    Events<ResultType> transformedBy(Function) const;

    template<typename ResultType,
             typename This        = Events<Ts...>,
             typename Object      = SingleTypeArgumentOf<This>,
             typename Requirement = IsObjectType<Object>>
    Events<ResultType> transformedBy(ResultType (Object::*)() const) const;

    template<typename Function, typename ResultType = TYPE_OF( VAL(Function)(VAL(Ts)...) )>
    Events<ResultType> transformedBy(const Variable<Function>&) const;

    template<typename Function, typename Requirement = HasSignature<Function, bool (Ts...)>>
    Events<Ts...> filteredBy(Function) const;

    template<typename This        = Events<Ts...>,
             typename Object      = SingleTypeArgumentOf<This>,
             typename Requirement = IsObjectType<Object>>
    Events<Object> filteredBy(bool (Object::*)() const) const;

    Events<Ts...> filteredBy(const Variable<bool>&) const;

    template<typename Function, typename Requirement = HasSignature<Function, bool (Ts...)>>
    Events<Ts...> filteredBy(const Variable<Function>&) const;

    template<typename Function, typename Requirement = HasSignature<Function, bool (Ts...)>>
    std::array<Events<Ts...>, 2> partitionedBy(Function) const;

    template<typename This        = Events<Ts...>,
             typename Object      = SingleTypeArgumentOf<This>,
             typename Requirement = IsObjectType<Object>>
    std::array<Events<Object>, 2> partitionedBy(bool (Object::*)() const) const;

    template<int n, typename Function, typename Requirement = HasSignature<Function, int (Ts...)>>
    std::array<Events<Ts...>, n> partitionedBy(Function) const;

    template<typename... Us>
    Events<Ts..., Us...> taggedWith(Us...);

    template<typename... Us>
    Events<Us..., Ts...> taggedAtFrontWith(Us...);

    template<typename... Us>
    Events<Ts..., Us...> withSnapshotOf(const Variable<Us>&...) const;

    template<typename... Us>
    Events<Us..., Ts...> withSnapshotAtFrontOf(const Variable<Us>&...) const;

    template<int... ns>
    WithPositions<Events<Ts...>, ns...> positions() const;

    template<int... ns>
    WithoutPositions<Events<Ts...>, ns...> withoutPositions() const;

    Events<> stripped() const;

    Events<std::tuple<Ts...>> tupled() const;

    template<typename This     = Events<Ts...>,
             typename Untupled = Untuple<This>>
    Untupled untupled() const; // Events<std::tuple<Ts...>> -> Events<Ts...>

    template<typename This            = Events<Ts...>,
             typename InnerEventsType = SingleTypeArgumentOf<This>,
             typename Requirement     = IsTypeInstanceOfTemplate<InnerEventsType, glaebhoerl::Events>> // glaebhoerl:: workaround
    InnerEventsType flattened() const;

    Variable<int> count() const;

    template<typename This = Events<Ts...>,
             typename T    = SingleTypeArgumentOf<This>>
    Variable<T> lastValueStartingWith(T) const;

    template<typename This = Events<Ts...>,
             typename T    = SingleTypeArgumentOf<This>>
    Variable<T> lastValueStartingWithDefault() const;

    template<int n,
             typename This = Events<Ts...>,
             typename T    = SingleTypeArgumentOf<This>>
    Variable<std::array<T, n>> historyStartingWith(std::array<T, n>) const;

    template<int n,
             typename This = Events<Ts...>,
             typename T    = SingleTypeArgumentOf<This>>
    Variable<std::array<T, n>> historyStartingWithDefaults() const;

    template<typename This = Events<Ts...>,
             typename T    = SingleTypeArgumentOf<This>>
    Variable<std::vector<T>> history(int) const;

    template<typename T, typename Function, typename Requirement = HasSignature<Function, T (T, Ts...)>>
    Variable<T> steppingFrom(T, Function) const;

    template<typename T,
             typename This        = Events<Ts...>,
             typename Function    = SingleTypeArgumentOf<This>,
             typename Requirement = HasSignature<Function, T (T)>>
    Variable<T> evolvingFrom(T) const;

    template<typename This        = Events<Ts...>,
             typename Action      = SingleTypeArgumentOf<This>,
             typename Requirement = HasSignature<Action, void ()>>
    void exec() const;


    template<typename U,
             typename This        = Events<Ts...>,
             typename T           = SingleTypeArgumentOf<This>,
             typename ElementType = TYPE_OF( VAL(T)[VAL(U)] )>
    Events<ElementType> operator [](U) const;

    template<typename U,
             typename This        = Events<Ts...>,
             typename T           = SingleTypeArgumentOf<This>,
             typename ElementType = TYPE_OF( VAL(T)[VAL(U)] )>
    Events<ElementType> operator [](const Variable<U>&) const;

    template<typename... Args,
             typename    This       = Events<Ts...>,
             typename    Function   = SingleTypeArgumentOf<This>,
             typename    ResultType = TYPE_OF( VAL(Function)(VAL(Args)...) )>
    Events<ResultType> operator ()(Args...) const;

    template<typename... Args,
             typename    This       = Events<Ts...>,
             typename    Function   = SingleTypeArgumentOf<This>,
             typename    ResultType = TYPE_OF( VAL(Function)(VAL(Args)...) )>
    Events<ResultType> operator ()(const Variable<Args>&...) const;

    template<typename Action, typename Requirement = HasSignature<Action, void (Ts...)>>
    void react(Action) const;

    template<typename Action, typename Requirement = HasSignature<Action, void (Ts...)>>
    std::function<void ()> reactUntilStopped(Action) const;

    void operator =(Events<Ts...>);

private:
    struct Impl;
    std::weak_ptr<Impl> m_impl;
};

#define DECLARE_PREFIX_OPERATOR_FOR_EVENT(OP)                      \
    template<typename T, typename ResultType = TYPE_OF(OP VAL(T))> \
    Events<ResultType> operator OP(const Events<T>&);              \

GLFRP_DO_FOR_EACH(DECLARE_PREFIX_OPERATOR_FOR_EVENT, +, -, !, ~, *)

#undef DECLARE_PREFIX_OPERATOR_FOR_EVENT

#define DECLARE_INFIX_OPERATORS_FOR_EVENT(OP)                                         \
    template<typename T, typename U, typename ResultType = TYPE_OF(VAL(T) OP VAL(U))> \
    Events<ResultType> operator OP(const Events<T>&, U);                              \
                                                                                      \
    template<typename T, typename U, typename ResultType = TYPE_OF(VAL(T) OP VAL(U))> \
    Events<ResultType> operator OP(const Events<T>&, const Variable<U>&);             \

GLFRP_DO_FOR_EACH(DECLARE_INFIX_OPERATORS_FOR_EVENT, +, -, *, /, %, ^, &, |, <<, >>, &&, ||, ==, !=, <, <=, >, >=)

#undef DECLARE_INFIX_OPERATORS_FOR_EVENT

template<typename Iterator,
         typename EventsType   = TYPE_OF(*VAL(Iterator)),
         typename Requirement  = IsTypeInstanceOfTemplate<EventsType, Events>,
         typename Requirements = IS_LEGAL(++VAL(Iterator&), VAL(Iterator) != VAL(Iterator))>
EventsType mergedEvents(Iterator begin, Iterator end);

template<typename Container, typename EventsType = TYPE_OF(mergedEvents(VAL(Container).cbegin(), VAL(Container).cend()))>
EventsType mergedEvents(const Container&);




template<typename... Ts>
struct Events<Ts...>::Generator
{
    Generator();
    Generator(const Generator&);

    void generate(Ts...) const;
    void operator ()(Ts...) const;

    void operator =(Generator);

private:
    std::shared_ptr<Impl> m_impl;
    friend class Events<Ts...>;
};

template<typename... Ts>
using EventGenerator = typename Events<Ts...>::Generator;

template<typename... Ts>
using EventGen = EventGenerator<Ts...>;




template<typename T>
struct Variable
{
    Variable();
    Variable(T);
    Variable(T, const Events<T>&);
    Variable(const Variable<T>&);

    template<typename Function, typename ResultType = TYPE_OF( VAL(Function)(VAL(T)) )>
    Variable<ResultType> transformedBy(Function) const;

    template<typename ResultType,
             typename Object      = T,
             typename Requirement = IsObjectType<Object>>
    Variable<ResultType> transformedBy(ResultType (Object::*)() const) const;

    template<typename Function, typename ResultType = TYPE_OF( VAL(Function)(VAL(T)) )>
    Variable<ResultType> transformedBy(const Variable<Function>&) const;

    template<typename InnerVariableType = T,
             typename Requirement       = IsTypeInstanceOfTemplate<InnerVariableType, glaebhoerl::Variable>> // glaebhoerl:: workaround
    InnerVariableType flattened() const;

    template<typename Action      = T,
             typename Requirement = HasSignature<Action, void ()>>
    void exec() const;

    template<typename U, typename ElementType = TYPE_OF( VAL(T)[VAL(U)] )>
    Variable<ElementType> operator [](U) const;

    template<typename U, typename ElementType = TYPE_OF( VAL(T)[VAL(U)] )>
    Variable<ElementType> operator [](const Variable<U>&) const;

    template<typename... Args, typename ResultType = TYPE_OF( VAL(T)(VAL(Args)...) )>
    Variable<ResultType> operator ()(Args...) const;

    template<typename... Args, typename ResultType = TYPE_OF( VAL(T)(VAL(Args)...) )>
    Variable<ResultType> operator ()(const Variable<Args>&...) const;

    Events<T> changes() const;

    T currentValue() const;

    template<typename Action, typename Requirement = HasSignature<Action, void (T)>>
    void always(Action) const;

    template<typename Action, typename Requirement = HasSignature<Action, void (T)>>
    std::function<void ()> alwaysUntilStopped(Action) const;

    void operator =(Variable<T>);

private:
    std::shared_ptr<T> m_value;
    Events<T> m_changes;
};

#define DECLARE_PREFIX_OPERATOR_FOR_VARIABLE(OP)                   \
    template<typename T, typename ResultType = TYPE_OF(OP VAL(T))> \
    Variable<ResultType> operator OP(const Variable<T>&);          \

GLFRP_DO_FOR_EACH(DECLARE_PREFIX_OPERATOR_FOR_VARIABLE, +, -, !, ~, *)

#undef DECLARE_PREFIX_OPERATOR_FOR_VARIABLE

#define DECLARE_INFIX_OPERATORS_FOR_VARIABLE(OP)                                      \
    template<typename T, typename U, typename ResultType = TYPE_OF(VAL(T) OP VAL(U))> \
    Variable<ResultType> operator OP(const Variable<T>&, U);                          \
                                                                                      \
    template<typename T, typename U, typename ResultType = TYPE_OF(VAL(T) OP VAL(U))> \
    Variable<ResultType> operator OP(T, const Variable<U>&);                          \
                                                                                      \
    template<typename T, typename U, typename ResultType = TYPE_OF(VAL(T) OP VAL(U))> \
    Variable<ResultType> operator OP(const Variable<T>&, const Variable<U>&);         \

GLFRP_DO_FOR_EACH(DECLARE_INFIX_OPERATORS_FOR_VARIABLE, +, -, *, /, %, ^, &, |, <<, >>, &&, ||, ==, !=, <, <=, >, >=)

#undef DECLARE_INFIX_OPERATORS_FOR_VARIABLE

template<typename T>
Variable<T> constant(T);

template<typename Function, typename ResultType = TYPE_OF( VAL(Function)() )>
Variable<ResultType> sample(Function, Events<>);

template<typename Function, typename... Ts, typename ResultType = TYPE_OF( VAL(Function)(VAL(Ts)...) )>
Variable<ResultType> transformedBy(Function, const Variable<Ts>&...);

template<typename... Ts>
Variable<std::tuple<Ts...>> tupled(const Variable<Ts>&...);

template<typename... Ts>
Events<Ts...> variableEvents(const Variable<Events<Ts...>>&);


template<typename T>
struct Mutable
{
    Mutable();
    Mutable(T);

    Variable<T> value() const;

    operator Variable<T>() const;

    T currentValue() const;

    Events<T> changes() const;

    void operator =(T) const;

    template<typename Action, typename Requirement = HasSignature<Action, void (T&)>>
    void mutate(const Action&) const;

    template<typename Object = T, typename Requirement = IsObjectType<Object>>
    void mutate(void (Object::*)()) const;

    template<typename Function, typename Requirement = HasSignature<Function, T (T)>>
    void modify(const Function&) const;

    template<typename Object = T, typename Requirement = IsObjectType<Object>>
    void modify(Object (Object::*)() const) const;

    template<typename This = T, typename Requirement = IS_LEGAL(++VAL(This&))>
    void operator ++() const;

    template<typename This = T, typename Requirement = IS_LEGAL(--VAL(This&))>
    void operator --() const;

    template<typename This = T, typename Requirement = IS_LEGAL(VAL(This&)++)>
    void operator ++(int) const;

    template<typename This = T, typename Requirement = IS_LEGAL(VAL(This&)--)>
    void operator --(int) const;

#define DECLARE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE(OP)                     \
    template<typename U, typename Requirement = IS_LEGAL(VAL(T&) OP VAL(U))> \
    void operator OP(U&&) const;                                             \

    GLFRP_DO_FOR_EACH(DECLARE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=)

#undef DECLARE_COMPOUND_ASSIGN_OPERATOR_FOR_MUTABLE

    Mutable(const Mutable<T>&);
    void operator =(Mutable<T>);

private:
    EventGen<T> m_update;
    Variable<T> m_value;
};

}

#undef VAL
#undef IS_LEGAL
#undef TYPE_OF

#include "FRP.hpp"

#endif

// TODO
//
// template<typename... Ts, typename T, typename State>
// Variable<T> steppingWithStateFrom(State, Event<Ts...>, std::tuple<State, T> (State, Ts...));
// mergedEvents(Events...)
// snapshotWith
// onOff
// integral
// in-place mutating steppingFrom, evolvingFrom...
// recursive
// continuous
// bind-to-position
