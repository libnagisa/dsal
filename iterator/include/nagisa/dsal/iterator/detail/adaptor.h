#pragma once

#include "./trait.h"
#include "./custom_point.h"
#include "./interface.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

/// \brief Iterator adapter, which provides an adapter to combine other methods of passing iterators
/// \tparam Iterator The type of the underlying iterator.
/// \note The functions assembled by the adapter depend on those provided by the Iterator. The dependencies are as follows
///		(iterator_adaptor a, Iterator i)
///		*a			-> *i
///		a += n		-> (i += n) | (i -= -n)
///		a -= n		-> (i -= n) | (a += -n)
///		++a			-> ++i | (a += 1)
///		a++			-> i++ | (++a, a = a)
///		--a			-> --i | (a += -1)
///		a--			-> i-- | (--a, a = a)
///		a + n		-> (i + n) | (a += n, a = a)
///		n + a		-> (n + i) | (a + n)
///		a[n]		-> (i[n]) | (a + n, *a)
///		a - n		-> (i - n) | (a + -n)
///		a - a		-> (i - i)
///		a <=> a		-> (i <=> i) | (a - a)
///		a == a		-> (i == i) | (a <=> a)
///		
///		concept						operations
///		input_or_output_iterator	movable, *i,	++i, difference_type
///		output_iterator				movable, *i = e,++i, difference_type
///		input_iterator				movable, *i,	++i, difference_type
///		forward_iterator			regular, *i,	++i, difference_type, i == i
///		bidirectional_iterator		regular, *i,	++i, difference_type, i == i, --i
///		random_access_iterator		regular, *i, i += n, i - i
///		contiguous_iterator			regular, *i, i += n, i - i
///	\TODO:
///		iter_swap
///		iter_move
template<class Iterator>
	requires ::std::is_object_v<Iterator>
struct iterator_adaptor final : iterator_interface<
	iterator_adaptor<Iterator>
	, iter_concept_t<Iterator>
	, iter_difference_t<Iterator>
	, iter_value_t<Iterator>
>{
private:
	using self_type = iterator_adaptor;
public:
	using iterator_type = Iterator;

	constexpr iterator_adaptor() noexcept(::std::is_nothrow_default_constructible_v<iterator_type>)
		requires ::std::constructible_from<iterator_type> = default;

	constexpr explicit(false) iterator_adaptor(auto&&... args)
		noexcept(::std::is_nothrow_constructible_v<iterator_type, decltype(args)...>)
		requires (sizeof...(args) != 0) && ::std::constructible_from<iterator_type, decltype(args)...>
	: _iter(::std::forward<decltype(args)>(args)...)
	{
	}

	constexpr decltype(auto) operator*() const noexcept(iter_cp::dereference<iterator_type>().is_nothrow)
		requires (iter_cp::dereference<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::dereference<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			return *_iter;
		}
	}
	constexpr decltype(auto) operator+=(iter_difference_t<iterator_type> n) noexcept(iter_cp::plus_eq<iterator_type>().is_nothrow)
		requires (iter_cp::plus_eq<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::plus_eq<iterator_type>();
		result.category == iter_cp::category::proxy)
			_iter += n;
		else if constexpr (result.category == iter_cp::category::_0)
			_iter -= -n;
		return *this;
	}
	constexpr decltype(auto) operator-=(iter_difference_t<iterator_type> n) noexcept(iter_cp::minus_eq<iterator_type>().is_nothrow)
		requires (iter_cp::minus_eq<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::minus_eq<iterator_type>();
			result.category == iter_cp::category::proxy)
			_iter -= n;
		else if constexpr (result.category == iter_cp::category::_0)
			return self_type::operator+=(-n);
		return *this;
	}
	constexpr decltype(auto) operator++() noexcept(iter_cp::weak_increment<iterator_type>().is_nothrow)
		requires (iter_cp::weak_increment<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::weak_increment<iterator_type>();
			result.category == iter_cp::category::proxy)
			++_iter;
		else if constexpr (result.category == iter_cp::category::_0)
			return self_type::operator+=(iter_difference_t<iterator_type>(1));
		return *this;
	}
	constexpr decltype(auto) operator++(int) noexcept(iter_cp::increment<iterator_type>().is_nothrow)
		requires (iter_cp::increment<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::increment<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			using result_type = decltype(_iter++);
			if constexpr (::std::constructible_from<self_type, result_type>)
				return self_type{ _iter++ };
			else
				return _iter++;
		}
		else if constexpr(result.category == iter_cp::category::_0)
		{
			if constexpr (::std::copy_constructible<iterator_type>)
			{
				auto ret = *this;
				self_type::operator++();
				return ret;
			}
			else
			{
				self_type::operator++();
			}
		}
	}
	constexpr decltype(auto) operator--() noexcept(iter_cp::weak_decrement<iterator_type>().is_nothrow)
		requires (iter_cp::weak_decrement<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::weak_decrement<iterator_type>();
			result.category == iter_cp::category::proxy)
			--_iter;
		else if constexpr (result.category == iter_cp::category::_0)
			return self_type::operator+=(iter_difference_t<iterator_type>(-1));
		return *this;
	}
	constexpr decltype(auto) operator--(int) noexcept(iter_cp::decrement<iterator_type>().is_nothrow)
		requires (iter_cp::decrement<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::decrement<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			using result_type = decltype(_iter--);
			if constexpr (::std::constructible_from<self_type, result_type>)
				return self_type{ _iter-- };
			else
				return _iter--;
		}
		else if constexpr (result.category == iter_cp::category::_0)
		{
			if constexpr (::std::copy_constructible<iterator_type>)
			{
				auto ret = *this;
				self_type::operator--();
				return ret;
			}
			else
			{
				self_type::operator--();
			}
		}
	}
	friend constexpr decltype(auto) operator+(::std::convertible_to<self_type const&> auto&& lhs, iter_difference_t<iterator_type> n)
		noexcept(iter_cp::left_plus<iterator_type>().is_nothrow)
		requires (iter_cp::left_plus<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::left_plus<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			decltype(auto) proxy_result = ::std::forward<decltype(lhs)>(lhs)._iter + n;
			if constexpr (::std::constructible_from<self_type, decltype(proxy_result)>)
				return self_type{ ::std::forward<decltype(proxy_result)>(proxy_result) };
			else
				return proxy_result;
		}
		else if constexpr (result.category == iter_cp::category::_0)
		{
			auto ret = ::std::forward<decltype(lhs)>(lhs);
			ret += n;
			return ret;
		}
	}
	friend constexpr decltype(auto) operator+(iter_difference_t<iterator_type> n, ::std::convertible_to<self_type const&> auto&& rhs)
		noexcept(iter_cp::right_plus<iterator_type>().is_nothrow)
		requires (iter_cp::right_plus<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::right_plus<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			decltype(auto) proxy_result = n + ::std::forward<decltype(rhs)>(rhs)._iter;
			if constexpr (::std::constructible_from<self_type, decltype(proxy_result)>)
				return self_type{ ::std::forward<decltype(proxy_result)>(proxy_result) };
			else
				return proxy_result;
		}
		else if constexpr (result.category == iter_cp::category::_0)
			return ::std::forward<decltype(rhs)>(rhs) + n;
	}
	constexpr decltype(auto) operator[](iter_difference_t<iterator_type> n) const noexcept(iter_cp::subscript<iterator_type>().is_nothrow)
		requires (iter_cp::subscript<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::subscript<iterator_type>();
			result.category == iter_cp::category::proxy)
			return _iter[n];
		else if constexpr (result.category == iter_cp::category::_0)
			return *(*this + n);
	}
	friend constexpr decltype(auto) operator-(::std::convertible_to<self_type const&> auto&& lhs, iter_difference_t<iterator_type> n)
		noexcept(iter_cp::left_minus<iterator_type>().is_nothrow)
		requires (iter_cp::left_minus<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::left_minus<iterator_type>();
			result.category == iter_cp::category::proxy)
		{
			decltype(auto) proxy_result = ::std::forward<decltype(lhs)>(lhs)._iter - n;
			if constexpr (::std::constructible_from<self_type, decltype(proxy_result)>)
				return self_type{ ::std::forward<decltype(proxy_result)>(proxy_result) };
			else
				return proxy_result;
		}
		else if constexpr (result.category == iter_cp::category::_0)
			return ::std::forward<decltype(lhs)>(lhs) + -n;
	}
	friend constexpr decltype(auto) operator-(::std::convertible_to<self_type const&> auto&& lhs, ::std::convertible_to<self_type const&> auto&& rhs)
		noexcept(iter_cp::minus<iterator_type>().is_nothrow)
		requires (iter_cp::minus<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::minus<iterator_type>();
			result.category == iter_cp::category::proxy)
			return ::std::forward<decltype(lhs)>(lhs)._iter - ::std::forward<decltype(rhs)>(rhs)._iter;
	}
	friend constexpr decltype(auto) operator<=>(::std::convertible_to<self_type const&> auto&& lhs, ::std::convertible_to<self_type const&> auto&& rhs)
		noexcept(iter_cp::three_way<iterator_type>().is_nothrow)
		requires (iter_cp::three_way<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::three_way<iterator_type>();
		result.category == iter_cp::category::proxy)
			return ::std::forward<decltype(lhs)>(lhs)._iter <=> ::std::forward<decltype(rhs)>(rhs)._iter;
		else if constexpr (result.category == iter_cp::category::_0)
			return ::std::forward<decltype(rhs)>(rhs) - ::std::forward<decltype(lhs)>(lhs);
	}
	friend constexpr decltype(auto) operator==(::std::convertible_to<self_type const&> auto&& lhs, ::std::convertible_to<self_type const&> auto&& rhs)
		noexcept(iter_cp::equal<iterator_type>().is_nothrow)
		requires (iter_cp::equal<iterator_type>().category != iter_cp::category::none)
	{
		if constexpr (constexpr iter_cp::result result = iter_cp::equal<iterator_type>();
		result.category == iter_cp::category::proxy)
			return ::std::forward<decltype(lhs)>(lhs)._iter == ::std::forward<decltype(rhs)>(rhs)._iter;
		else if constexpr (result.category == iter_cp::category::_0)
			return (::std::forward<decltype(lhs)>(lhs) - ::std::forward<decltype(rhs)>(rhs)) == 0;
		else if constexpr (result.category == iter_cp::category::_1)
			return (::std::forward<decltype(lhs)>(lhs) <=> ::std::forward<decltype(rhs)>(rhs)) == ::std::strong_ordering::equal;
	}

	iterator_type _iter;
};

NAGISA_BUILD_LIB_DETAIL_END