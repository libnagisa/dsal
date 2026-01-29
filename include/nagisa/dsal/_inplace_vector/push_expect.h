#pragma push_macro("NAGISA_DSAL_EXPECT")
#if __cpp_contracts >= 202502l
#	define NAGISA_DSAL_EXPECT(condition, message) contract_assert(condition)
#elifndef NDEBUG
#	define NAGISA_DSAL_EXPECT(condition, message)	\
	do												\
	{												\
		if (!(condition))							\
			::std::abort();							\
	} while (false)									\
//
#else
#define NAGISA_DSAL_EXPECT(condition, message) (static_cast<void>(0))
#endif