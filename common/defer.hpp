#pragma once
#ifndef E_DEFER_HPP
#define E_DEFER_HPP

#define P_DEFER_CONCATENATE_IMPL(x, y) x##y
#define P_DEFER_CONCATENATE(x, y)      P_DEFER_CONCATENATE_IMPL(x, y)

template <typename F>
struct P_Deferral {
	F f;
    P_Deferral(F f): f(f) {}
	~P_Deferral()  { f(); }
};

template <typename F>
inline static P_Deferral<F> p_deferral_create(F f) {
	return P_Deferral<F>(f);
}

#define P_DEFER_IMPL(x) P_DEFER_CONCATENATE(x, __COUNTER__)
#define DEFER(code)     auto P_DEFER_IMPL(_deferral_) = p_deferral_create([&](){ code; })

#endif // E_DEFER_HPP
