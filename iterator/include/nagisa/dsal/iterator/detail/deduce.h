#pragma once

#include "./interface.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<
	class IteratorConcept,
	class ValueType,
	class Reference = ValueType&,
	class Pointer = ValueType*,
	class DifferenceType = ::std::ptrdiff_t>
struct deduced_iterator_interface : iterator_category_base<IteratorConcept, Reference>
{
	using iterator_concept = IteratorConcept;
	using value_type = ::std::remove_cv_t<ValueType>;
	using reference = Reference;
	using pointer = pointer_t<Pointer, iterator_concept>;
	using difference_type = DifferenceType;

	constexpr decltype(auto) operator*(this auto&& self)
		requires requires { *access::base(self); }
	{
		return *access::base(self);
	}

	constexpr auto operator->(this auto&& self)
		requires (!std::same_as<pointer, void>) && std::is_reference_v<reference>&& requires { *self; }
	{
		return detail::make_pointer<pointer, reference>(*self);
	}

	constexpr decltype(auto) operator[](this auto const& self, difference_type n)
		requires requires { self + n; } {
		auto retval = self;
		retval = retval + n;
		return *retval;
	}

	constexpr decltype(auto) operator++(this auto& self)
		requires requires { ++access::base(self); } && (!requires { self += difference_type(1); }) {
		++access::base(self);
		return self;
	}
	constexpr decltype(auto) operator++(this auto& self)
		requires requires { self += difference_type(1); } {
		return self += difference_type(1);
	}
	constexpr auto operator++(this auto& self, int) requires requires { ++self; } {
		if constexpr (std::is_same_v<IteratorConcept, std::input_iterator_tag>) {
			++self;
		}
		else {
			auto retval = self;
			++self;
			return retval;
		}
	}
	constexpr decltype(auto) operator+=(this auto& self, difference_type n)
		requires requires { access::base(self) += n; } {
		access::base(self) += n;
		return self;
	}

	constexpr decltype(auto) operator--(this auto& self)
		requires requires { --access::base(self); } && (!requires { self += difference_type(1); }) {
		--access::base(self);
		return self;
	}
	constexpr decltype(auto) operator--(this auto& self)
		requires requires { self += -difference_type(1); } {
		return self += -difference_type(1);
	}
	constexpr auto operator--(this auto& self, int) requires requires { --self; } {
		auto retval = self;
		--self;
		return retval;
	}
	constexpr decltype(auto) operator-=(this auto& self, difference_type n)
		requires requires { self += -n; } {
		return self += -n;
	}
};

namespace boost {
	namespace stl_interfaces {
		BOOST_STL_INTERFACES_NAMESPACE_V3{

			// clang-format off

			/** A base template that one may derive from to make defining iterators
				easier. */
			

  namespace v3_dtl {
	  template<
		  typename IteratorConcept,
		  typename ValueType,
		  typename Reference,
		  typename Pointer,
		  typename DifferenceType>
	  void derived_iterator(v3::iterator_interface<
							IteratorConcept,
							ValueType,
							Reference,
							Pointer,
							DifferenceType> const&);

	  template<typename D>
	  concept derived_iter = requires (D d) { v3_dtl::derived_iterator(d); };
  }

  template<typename D>
	constexpr auto operator+(D it, typename D::difference_type n)
	  requires v3_dtl::derived_iter<D>&& requires { it += n; }
		{ return it += n; }
  template<typename D>
	constexpr auto operator+(typename D::difference_type n, D it)
	  requires v3_dtl::derived_iter<D>&& requires { it += n; }
		{ return it += n; }

  template<typename D1, typename D2>
	constexpr auto operator-(D1 lhs, D2 rhs)
	  requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&&
			   requires { access::base(lhs) - access::base(rhs); }
		{ return access::base(lhs) - access::base(rhs); }
  template<typename D>
	constexpr auto operator-(D it, typename D::difference_type n)
	  requires v3_dtl::derived_iter<D>&& requires { it += -n; }
		{ return it += -n; }

#if defined(__cpp_lib_three_way_comparison)
	template<typename D1, typename D2>
	  constexpr auto operator<=>(D1 lhs, D2 rhs)
		requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2> &&
		(v2::v2_dtl::base_3way<D1, D2> || v2::v2_dtl::iter_sub<D1, D2>) {
		if constexpr (v2::v2_dtl::base_3way<D1, D2>) {
			return access::base(lhs) <=> access::base(rhs);
		  }
 else {
using diff_type = typename D1::difference_type;
diff_type const diff = rhs - lhs;
return diff < diff_type(0) ? std::strong_ordering::less :
  diff_type(0) < diff ? std::strong_ordering::greater :
  std::strong_ordering::equal;
}
}
#endif
	template<typename D1, typename D2>
	  constexpr bool operator<(D1 lhs, D2 rhs)
		requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&& v2::v2_dtl::iter_sub<D1, D2>
		  {
 return (lhs - rhs) < typename D1::difference_type(0);
}
template<typename D1, typename D2>
  constexpr bool operator<=(D1 lhs, D2 rhs)
	requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&& v2::v2_dtl::iter_sub<D1, D2>
	  {
return (lhs - rhs) <= typename D1::difference_type(0);
}
template<typename D1, typename D2>
  constexpr bool operator>(D1 lhs, D2 rhs)
	requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&& v2::v2_dtl::iter_sub<D1, D2>
	  {
return (lhs - rhs) > typename D1::difference_type(0);
}
template<typename D1, typename D2>
  constexpr bool operator>=(D1 lhs, D2 rhs)
	requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&& v2::v2_dtl::iter_sub<D1, D2>
	  {
return (lhs - rhs) >= typename D1::difference_type(0);
}

template<typename D1, typename D2>
  constexpr bool operator==(D1 lhs, D2 rhs)
	requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>&&
			 detail::interoperable<D1, D2>::value &&
	(v2::v2_dtl::base_eq<D1, D2> || v2::v2_dtl::iter_sub<D1, D2>) {
	if constexpr (v2::v2_dtl::base_eq<D1, D2>) {
	  return (access::base(lhs) == access::base(rhs));
	}
else if constexpr (v2::v2_dtl::iter_sub<D1, D2>) {
return (lhs - rhs) == typename D1::difference_type(0);
}
}

template<typename D1, typename D2>
  constexpr auto operator!=(D1 lhs, D2 rhs) -> decltype(!(lhs == rhs))
	requires v3_dtl::derived_iter<D1>&& v3_dtl::derived_iter<D2>
	  {
return !(lhs == rhs);
}

	  // clang-format on


	  /** A template alias useful for defining proxy iterators.  \see
		  `iterator_interface`. */
	  template<
		  typename IteratorConcept,
		  typename ValueType,
		  typename Reference = ValueType,
		  typename DifferenceType = std::ptrdiff_t>
	  using proxy_iterator_interface = iterator_interface<
		  IteratorConcept,
		  ValueType,
		  Reference,
		  proxy_arrow_result<Reference>,
		  DifferenceType>;

		}
	}
}

NAGISA_BUILD_LIB_DETAIL_END