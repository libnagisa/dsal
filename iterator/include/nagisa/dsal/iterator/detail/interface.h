#pragma once

#include "./base.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<
	class D,
	class IteratorConcept,
	class ValueType,
	class DifferenceType = ::std::ptrdiff_t>
	requires ::std::is_class_v<D> && ::std::same_as<D, ::std::remove_cv_t<D>>
struct iterator_interface : iterator_base<DifferenceType>
{
private:
	constexpr auto&& derived() noexcept { return static_cast<D&>(*this); }
	constexpr auto&& derived() const noexcept { return static_cast<D const&>(*this); }
public:
	using iterator_concept = IteratorConcept;
	using value_type = std::remove_cv_t<ValueType>;

	constexpr auto operator->() noexcept(noexcept(::std::addressof(*derived())))
		requires ::std::is_reference_v<::std::iter_reference_t<D>>
	{
		return ::std::addressof(*derived());
	}
	constexpr auto operator->() const noexcept(noexcept(::std::addressof(*derived())))
		requires ::std::is_reference_v<::std::iter_reference_t<D>>
	{
		return ::std::addressof(*derived());
	}

	constexpr decltype(auto) operator[](::std::iter_difference_t<D> n) const noexcept(*(derived() + n))
		requires requires (D const d) { d + n; }
	{
		return *(derived() + n);
	}
	constexpr decltype(auto) operator++() noexcept(noexcept(derived() += ::std::iter_difference_t<D>(1)))
		requires requires (D d) { d += ::std::iter_difference_t<D>(1); }
	{
		return derived() += ::std::iter_difference_t<D>(1);
	}
	constexpr auto operator++(int)
		noexcept(noexcept(++derived()) 
			&& ((!::std::derived_from<iterator_concept, ::std::forward_iterator_tag> && !::std::same_as<iterator_concept, ::std::output_iterator_tag>) || ::std::is_nothrow_copy_constructible_v<D>))
		requires requires (D d) { ++d; }
	{
		if constexpr(::std::derived_from<iterator_concept, ::std::forward_iterator_tag> || ::std::same_as<iterator_concept, ::std::output_iterator_tag>)
		{
			auto result = derived();
			++derived();
			return result;
		}
		else
		{
			++derived();
		}
	}
	constexpr decltype(auto) operator--() noexcept(noexcept(derived() += -::std::iter_difference_t<D>(1)))
		requires requires (D d) { d += -::std::iter_difference_t<D>(1); }
	{
		return derived() += -::std::iter_difference_t<D>(1);
	}
	constexpr auto operator--(int) noexcept(noexcept(--derived()) && ::std::is_nothrow_copy_constructible_v<D>)
		requires requires (D d) { --d; }
	{
		auto result = derived();
		--derived();
		return result;
	}
	constexpr decltype(auto) operator-=(::std::iter_difference_t<D> n) noexcept(noexcept(derived() += -n))
		requires requires (D d) { d += -n; }
	{
		return derived() += -n;
	}
};

NAGISA_BUILD_LIB_DETAIL_END

