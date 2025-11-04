#pragma once

#include "./trait.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

namespace iter_cp
{
	enum class category
	{
		none,
		proxy,
		_0,
		_1,
		_2,
	};
	struct result
	{
		category category = category::none;
		bool is_nothrow = false;
	};
	inline constexpr auto fail = result{ category::none, false };
#define NAGISA_DSAL_ITERATOR_OPERATION(name, params, statement)						\
	template<class T>																\
	concept can_##name = requires params { statement; };							\
	//
	NAGISA_DSAL_ITERATOR_OPERATION(dereference, (T i), *i);
	NAGISA_DSAL_ITERATOR_OPERATION(weak_increment, (T i), ++i);
	NAGISA_DSAL_ITERATOR_OPERATION(increment, (T i), i++);
	NAGISA_DSAL_ITERATOR_OPERATION(weak_decrement, (T i), --i);
	NAGISA_DSAL_ITERATOR_OPERATION(decrement, (T i), i--);
	NAGISA_DSAL_ITERATOR_OPERATION(plus_eq, (T i, iter_difference_t<T> n), i += n);
	NAGISA_DSAL_ITERATOR_OPERATION(minus_eq, (T i, iter_difference_t<T> n), i -= n);
	NAGISA_DSAL_ITERATOR_OPERATION(subscript, (T i, iter_difference_t<T> n), i[n]);
	NAGISA_DSAL_ITERATOR_OPERATION(left_plus, (T i, iter_difference_t<T> n), i + n);
	NAGISA_DSAL_ITERATOR_OPERATION(right_plus, (T i, iter_difference_t<T> n), n + i);
	NAGISA_DSAL_ITERATOR_OPERATION(left_minus, (T i, iter_difference_t<T> n), i - n);
	NAGISA_DSAL_ITERATOR_OPERATION(minus, (T i, T j), i - j);
	NAGISA_DSAL_ITERATOR_OPERATION(three_way, (T i, T j), i <=> j);
	NAGISA_DSAL_ITERATOR_OPERATION(equal, (T i, T j), i == j);
	//NAGISA_DSAL_ITERATOR_OPERATION(not_equal, (T i, T j), i != j);
	//NAGISA_DSAL_ITERATOR_OPERATION(less, (T i, T j), i < j);
	//NAGISA_DSAL_ITERATOR_OPERATION(less_equal, (T i, T j), i <= j);
	//NAGISA_DSAL_ITERATOR_OPERATION(greater, (T i, T j), i > j);
	//NAGISA_DSAL_ITERATOR_OPERATION(greater_equal, (T i, T j), i >= j);
#undef NAGISA_DSAL_ITERATOR_OPERATION

#define NAGISA_DSAL_ITERATOR_CALL_IF_ENABLE(name, params, ...)	\
	if constexpr (can_##name<T> __VA_ARGS__)					\
		indirectly_##name params								\
		//

	template<class T>
	concept nothrow_copy_constructable_or_moveonly = !::std::copy_constructible<T> || ::std::is_nothrow_copy_constructible_v<T>;

	template<class T>
	consteval result dereference() noexcept
	{
		if constexpr (can_dereference<T>)
			return { category::proxy, noexcept(*::std::declval<T&>()) };
		else
			return fail;
	}
	template<class T>
	consteval result plus_eq() noexcept
	{
		if constexpr (can_plus_eq<T>)
			return { category::proxy, noexcept(::std::declval<T&>() += ::std::declval<iter_difference_t<T>>()) };
		else if constexpr (can_minus_eq<T>)
			return { category::_0, noexcept(::std::declval<T&>() -= -::std::declval<iter_difference_t<T>>()) };
		else
			return fail;
	}
	template<class T>
	consteval result minus_eq() noexcept
	{
		constexpr result r = iter_cp::plus_eq<T>();
		if constexpr (can_minus_eq<T>)
			return { category::proxy, noexcept(::std::declval<T&>() -= ::std::declval<iter_difference_t<T>>()) };
		else if constexpr (r.category != category::none)
			return { category::_0,  r.is_nothrow && noexcept(-::std::declval<iter_difference_t<T>>()) };
		else
			return fail;
	}
	template<class T>
	consteval result weak_increment() noexcept
	{
		constexpr result r = iter_cp::plus_eq<T>();
		if constexpr (can_weak_increment<T>)
			return { category::proxy, noexcept(++::std::declval<T&>()) };
		else if constexpr (r.category != category::none 
			&& ::std::constructible_from<iter_difference_t<T>, int>)
			return { category::_0, r.is_nothrow && ::std::is_nothrow_constructible_v<iter_difference_t<T>, int> };
		else
			return fail;
	}
	template<class T>
	consteval result increment() noexcept
	{
		constexpr result r = iter_cp::weak_increment<T>();
		if constexpr (can_increment<T>)
			return { category::proxy, noexcept(::std::declval<T&>()++) };
		else if constexpr (r.category != category::none)
			return { category::_0, r.is_nothrow && nothrow_copy_constructable_or_moveonly<T> };
		else
			return fail;
	}
	template<class T>
	consteval result weak_decrement() noexcept
	{
		constexpr result r = iter_cp::plus_eq<T>();
		if constexpr (can_weak_decrement<T>)
			return { category::proxy, noexcept(--::std::declval<T&>()) };
		else if constexpr (r.category != category::none
			&& ::std::constructible_from<iter_difference_t<T>, int>)
			return { category::_0,  r.is_nothrow && ::std::is_nothrow_constructible_v<iter_difference_t<T>, int> };
		else
			return fail;
	}
	template<class T>
	consteval result decrement() noexcept
	{
		constexpr result r = iter_cp::weak_decrement<T>();
		if constexpr (can_decrement<T>)
			return { category::proxy, noexcept(::std::declval<T&>()--) };
		else if constexpr (r.category != category::none)
			return { category::_0, r.is_nothrow && nothrow_copy_constructable_or_moveonly<T> };
		else
			return fail;
	}
	template<class T>
	consteval result left_plus() noexcept
	{
		constexpr result r = iter_cp::plus_eq<T>();
		if constexpr (can_left_plus<T>)
			return { category::proxy, noexcept(::std::declval<T&>() + ::std::declval<iter_difference_t<T>>()) };
		else if constexpr (r.category != category::none
			&& ::std::copy_constructible<T>)
			return { category::_0, r.is_nothrow && ::std::is_nothrow_copy_constructible_v<T> };
		else
			return fail;
	}
	template<class T>
	consteval result right_plus() noexcept
	{
		constexpr result r = iter_cp::left_plus<T>();
		if constexpr (can_right_plus<T>)
			return { category::proxy, noexcept(::std::declval<iter_difference_t<T>>() + ::std::declval<T&>()) };
		else if constexpr (r.category != category::none)
			return { category::_0, r.is_nothrow };
		else
			return fail;
	}
	template<class T>
	consteval result subscript() noexcept
	{
		constexpr result p = iter_cp::left_plus<T>(), d = iter_cp::dereference<T>();
		if constexpr (can_subscript<T>)
			return { category::proxy, noexcept(::std::declval<T&>()[::std::declval<iter_difference_t<T>>()]) };
		else if constexpr (p.category != category::none && d.category != category::none)
			return { category::_0, p.is_nothrow && d.is_nothrow };
		else
			return fail;
	}
	template<class T>
	consteval result left_minus() noexcept
	{
		constexpr result r = iter_cp::left_plus<T>();
		if constexpr (can_left_minus<T>)
			return { category::proxy, noexcept(::std::declval<T&>() - ::std::declval<iter_difference_t<T>>()) };
		else if constexpr (r.category != category::none)
			return { category::_0, r.is_nothrow && noexcept(-::std::declval<iter_difference_t<T>>()) };
		else
			return fail;
	}
	template<class T>
	consteval result minus() noexcept
	{
		if constexpr (can_minus<T>)
			return { category::proxy, noexcept(::std::declval<T&>() - ::std::declval<T&>()) };
		else
			return fail;
	}
	template<class T>
	consteval result three_way() noexcept
	{
		constexpr result r = iter_cp::minus<T>();
		if constexpr (can_three_way<T>)
			return { category::proxy, noexcept(::std::declval<T&>() <=> ::std::declval<T&>()) };
		else if constexpr (r.category != category::none)
			return { category::_0, r.is_nothrow };
		else
			return fail;
	}
	template<class T>
	consteval result equal() noexcept
	{
		constexpr result mr = iter_cp::minus<T>(), tr = iter_cp::three_way<T>();
		if constexpr (can_equal<T>)
			return { category::proxy, noexcept(::std::declval<T&>() == ::std::declval<T&>()) };
		else if constexpr (mr.category != category::none
			&& ::std::equality_comparable_with<iter_difference_t<T>, int>)
			return { category::_0, mr.is_nothrow && noexcept(::std::declval<iter_difference_t<T>>() == ::std::declval<int>()) };
		else if constexpr (tr.category != category::none
			&& ::std::three_way_comparable<T, ::std::strong_ordering>)
			return { category::_1, tr.is_nothrow && noexcept((::std::declval<T&>() <=> ::std::declval<T&>()) == ::std::declval<::std::strong_ordering>()) };
		else
			return fail;
	}
}

NAGISA_BUILD_LIB_DETAIL_END