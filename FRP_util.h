#ifndef GLAEBHOERL_FRP_UTIL_H
#define GLAEBHOERL_FRP_UTIL_H

#include <tuple>

#define GLFRP_VAL(TYPE) std::declval<TYPE>()

namespace glaebhoerl
{

namespace Impl
{
    template<typename>
    struct SingleTypeArgumentOf { };

    template<template<typename...> class F, typename T>
    struct SingleTypeArgumentOf<F<T>>
    {
        using Type = T;
    };

    template<typename>
    struct Untuple { };

    template<template<typename...> class F, typename... Ts>
    struct Untuple<F<std::tuple<Ts...>>>
    {
        using Type = F<Ts...>;
    };

    template<typename, int>
    struct AtPosition { };

    template<template<typename...> class F, typename T, typename... Ts>
    struct AtPosition<F<T, Ts...>, 0>
    {
        using Type = T;
    };

    template<template<typename...> class F, typename T, typename... Ts, int n>
    struct AtPosition<F<T, Ts...>, n>
    {
        using Type = typename AtPosition<F<Ts...>, n - 1>::Type;
        static_assert(0 <= n && n <= sizeof...(Ts), "");
    };

    template<typename, int...>
    struct WithPositions { };

    template<template<typename...> class F, typename... Ts, int... ns>
    struct WithPositions<F<Ts...>, ns...>
    {
        using Type = F<typename AtPosition<F<Ts...>, ns>::Type...>;
    };

    template<typename, int...>
    struct WithoutPositions { }; // TODO

    template<typename>
    struct ToIndices { };

    template<typename A, typename B>
    struct Equals { };

    template<typename T>
    struct Equals<T, T>
    {
        using Type = void;
    };

    template<typename Type, template<typename...> class Template>
    struct IsTypeInstanceOfTemplate { };

    template<template<typename...> class Template, typename... Ts>
    struct IsTypeInstanceOfTemplate<Template<Ts...>, Template>
    {
        using Type = void;
    };

    template<typename T>
    struct WithoutQualifiers
    {
        using Type = T;
    };

    template<typename T>
    struct WithoutQualifiers<T&>
    {
        using Type = T;
    };

    template<typename T>
    struct WithoutQualifiers<const T&>
    {
        using Type = T;
    };

    template<typename T>
    struct WithoutQualifiers<T&&>
    {
        using Type = T;
    };

    template<typename T>
    struct WithoutQualifiers<const T&&>
    {
        using Type = T;
    };

    template<typename Signature>
    struct SignatureTester { };

    template<typename Ret, typename... Args>
    struct SignatureTester<Ret (Args...)>
    {
        template<typename Function, typename Result = decltype(GLFRP_VAL(Function)(GLFRP_VAL(Args)...))>
        struct Test { };
    };

    template<typename Ret, typename... Args>
    template<typename Function>
    struct SignatureTester<Ret (Args...)>::Test<Function, Ret>
    {
        using Type = void;
    };
}

template<typename T>
using SingleTypeArgumentOf = typename Impl::SingleTypeArgumentOf<T>::Type;

template<typename T>
using Untuple = typename Impl::Untuple<T>::Type;

template<typename T, int... ns>
using WithPositions = typename Impl::WithPositions<T, ns...>::Type;

template<typename T, int... ns>
using WithoutPositions = typename Impl::WithoutPositions<T, ns...>::Type;

template<typename T>
using ToIndices = typename Impl::ToIndices<T>::Type;

template<typename A, typename B>
using Equals = typename Impl::Equals<A, B>::Type;

template<typename Function, typename Signature>
using HasSignature = typename Impl::SignatureTester<Signature>::template Test<Function>::Type;

template<typename T>
using IsObjectType = decltype( (GLFRP_VAL(T).*GLFRP_VAL(void (T::*)()))() );

template<typename Type, template<typename...> class Template>
using IsTypeInstanceOfTemplate = typename Impl::IsTypeInstanceOfTemplate<Type, Template>::Type;

template<typename T>
using WithoutQualifiers = typename Impl::WithoutQualifiers<T>::Type;

}

#define GLFRP_DO_FOR_EACH(MACRO, ...) IMPL_GLFRP_CAT(IMPL_GLFRP_DO_FOR_EACH_, GLFRP_COUNT_ARGS(__VA_ARGS__))(MACRO, __VA_ARGS__)

#define GLFRP_COUNT_ARGS(...) IMPL_GLFRP_COUNT_ARGS(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
                                                           48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, \
                                                           32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
                                                           16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1)

#define IMPL_GLFRP_COUNT_ARGS( _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, _11, _12, _13, _14, _15, _16, \
                           _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
                           _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
                           _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, \
                           COUNT, ...) COUNT

#define IMPL_GLFRP_CAT(ARG1,  ARG2) IMPL_GLFRP_CAT2(ARG1, ARG2)
#define IMPL_GLFRP_CAT2(ARG1, ARG2) ARG1 ## ARG2


#define IMPL_GLFRP_DO_FOR_EACH_1(MACRO,  ARG, ...) MACRO(ARG)
#define IMPL_GLFRP_DO_FOR_EACH_2(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_1(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_3(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_2(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_4(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_3(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_5(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_4(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_6(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_5(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_7(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_6(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_8(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_7(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_9(MACRO,  ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_8(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_10(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_9(MACRO,  __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_11(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_10(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_12(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_11(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_13(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_12(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_14(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_13(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_15(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_14(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_16(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_15(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_17(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_16(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_18(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_17(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_19(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_18(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_20(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_19(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_21(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_20(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_22(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_21(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_23(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_22(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_24(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_23(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_25(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_24(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_26(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_25(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_27(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_26(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_28(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_27(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_29(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_28(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_30(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_29(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_31(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_30(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_32(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_31(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_33(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_32(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_34(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_33(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_35(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_34(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_36(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_35(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_37(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_36(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_38(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_37(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_39(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_38(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_40(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_39(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_41(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_40(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_42(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_41(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_43(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_42(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_44(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_43(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_45(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_44(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_46(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_45(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_47(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_46(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_48(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_47(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_49(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_48(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_50(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_49(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_51(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_50(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_52(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_51(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_53(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_52(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_54(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_53(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_55(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_54(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_56(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_55(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_57(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_56(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_58(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_57(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_59(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_58(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_60(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_59(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_61(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_60(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_62(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_61(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_63(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_62(MACRO, __VA_ARGS__)
#define IMPL_GLFRP_DO_FOR_EACH_64(MACRO, ARG, ...) MACRO(ARG) IMPL_GLFRP_DO_FOR_EACH_63(MACRO, __VA_ARGS__)


#endif

