#ifndef _BITS_STRTOT_HPP
#define _BITS_STRTOT_HPP 1

#include <BAN/Math.h>

#include <errno.h>
#include <ctype.h>
#include <wchar.h>

template<typename CHAR>
struct strtoT_traits;

template<>
struct strtoT_traits<char>
{
	static constexpr int (&isalnum)(int)  = ::isalnum;
	static constexpr int (&isalpha)(int)  = ::isalpha;
	static constexpr int (&isdigit)(int)  = ::isdigit;
	static constexpr int (&isspace)(int)  = ::isspace;
	static constexpr int (&isxdigit)(int) = ::isxdigit;
	static constexpr int (&tolower)(int)  = ::tolower;
	static constexpr int (&strncasecmp)(const char*, const char*, size_t) = ::strncasecmp;
	static constexpr const char* lit_0b       = "0b";
	static constexpr const char* lit_0x       = "0x";
	static constexpr const char* lit_inf      = "inf";
	static constexpr const char* lit_infinity = "infinity";
	static constexpr const char* lit_nan      = "nan";
};

template<>
struct strtoT_traits<wchar_t>
{
	static constexpr int (&isalnum)(wint_t)    = ::iswalnum;
	static constexpr int (&isalpha)(wint_t)    = ::iswalpha;
	static constexpr int (&isdigit)(wint_t)    = ::iswdigit;
	static constexpr int (&isspace)(wint_t)    = ::iswspace;
	static constexpr int (&isxdigit)(wint_t)   = ::iswxdigit;
	static constexpr wint_t (&tolower)(wint_t) = ::towlower;
	static constexpr int (&strncasecmp)(const wchar_t*, const wchar_t*, size_t) = ::wcsncasecmp;
	static constexpr const wchar_t* lit_0x       = L"0x";
	static constexpr const wchar_t* lit_inf      = L"inf";
	static constexpr const wchar_t* lit_infinity = L"infinity";
	static constexpr const wchar_t* lit_nan      = L"nan";
};

template<BAN::integral T, typename CHAR>
static T strtoT(const CHAR* str, CHAR** endp, int base, int& error)
{
	using traits = strtoT_traits<CHAR>;

	constexpr auto will_digit_append_overflow = [](T current, int digit, int base) -> bool {
		if (BAN::Math::will_multiplication_overflow<T>(current, base))
			return true;
		if (BAN::Math::will_addition_overflow<T>(current * base, current < 0 ? -digit : digit))
			return true;
		return false;
	};

	constexpr auto get_base_digit = [](CHAR c, int base) {
		int digit = -1;
		if (traits::isdigit(c))
			digit = c - CHAR('0');
		else if (traits::isalpha(c))
			digit = 10 + traits::tolower(c) - CHAR('a');
		if (digit < 0 || digit >= base)
			digit = -1;
		return digit;
	};

	const CHAR* orig_str = str;

	// validate base
	if (base != 0 && (base < 2 || base > 36))
	{
		if (endp)
			*endp = const_cast<CHAR*>(str);
		error = EINVAL;
		return 0;
	}

	// skip whitespace
	while (traits::isspace(*str))
		str++;

	// get sign and skip it
	const bool negative = (*str == CHAR('-'));
	if (*str == CHAR('-') || *str == CHAR('+'))
		str++;

	// determine base from prefix
	if (base == 0)
	{
		if (*str != CHAR('0'))
			base = 10;
		else if (traits::tolower(str[1]) == CHAR('x'))
			base = 16;
		else
			base = 8;
	}

	// check for invalid conversion
	if (get_base_digit(*str, base) == -1)
	{
		if (endp)
			*endp = const_cast<CHAR*>(orig_str);
		error = EINVAL;
		return 0;
	}

	// remove "0x" prefix from hexadecimal
	if (base == 16 && traits::strncasecmp(str, traits::lit_0x, 2) == 0 && get_base_digit(str[2], base) != -1)
		str += 2;

	bool overflow = false;

	T result = 0;
	// calculate the value of the number in string
	while (!overflow)
	{
		int digit = get_base_digit(*str, base);
		if (digit == -1)
			break;
		str++;

		overflow = will_digit_append_overflow(result, digit, base);
		if (!overflow)
		{
			if (negative && !BAN::is_unsigned_v<T>)
				digit = -digit;
			result = result * base + digit;
		}
	}

	if (negative && BAN::is_unsigned_v<T>)
		result = -result;

	// save endp if asked
	if (endp)
	{
		while (get_base_digit(*str, base) != -1)
			str++;
		*endp = const_cast<CHAR*>(str);
	}

	// return error on overflow
	if (overflow)
	{
		error = ERANGE;
		if constexpr(BAN::is_unsigned_v<T>)
			return BAN::numeric_limits<T>::max();
		return negative ? BAN::numeric_limits<T>::min() : BAN::numeric_limits<T>::max();
	}

	return result;
}

template<BAN::floating_point T, typename CHAR>
static T strtoT(const CHAR* str, CHAR** endp, int& error)
{
	using traits = strtoT_traits<CHAR>;

	constexpr auto get_base_digit = [](CHAR c, int base) {
		int digit = -1;
		if (traits::isdigit(c))
			digit = c - CHAR('0');
		else if (traits::isalpha(c))
			digit = 10 + traits::tolower(c) - CHAR('a');
		if (digit < 0 || digit >= base)
			digit = -1;
		return digit;
	};

	// find nan end including possible n-char-sequence
	const auto get_nan_end = [](const CHAR* str) -> const CHAR* {
		ASSERT(traits::strncasecmp(str, traits::lit_nan, 3) == 0);
		if (str[3] != CHAR('('))
			return str + 3;
		for (size_t i = 4; traits::isalnum(str[i]) || str[i] == CHAR('_'); i++)
			if (str[i] == CHAR(')'))
				return str + i + 1;
		return str + 3;
	};

	// skip whitespace
	while (traits::isspace(*str))
		str++;

	// get sign and skip it
	const bool negative = (*str == CHAR('-'));
	if (*str == CHAR('-') || *str == CHAR('+'))
		str++;

	// check for infinity or nan
	{
		T result = 0;

		if (traits::strncasecmp(str, traits::lit_inf, 3) == 0)
		{
			result = BAN::numeric_limits<T>::infinity();
			str += traits::strncasecmp(str, traits::lit_infinity, 8) ? 3 : 8;
		}
		else if (traits::strncasecmp(str, traits::lit_nan, 3) == 0)
		{
			result = BAN::numeric_limits<T>::quiet_NaN();
			str = get_nan_end(str);
		}

		if (result != 0)
		{
			if (endp)
				*endp = const_cast<CHAR*>(str);
			return negative ? -result : result;
		}
	}

	// no conversion can be performed -- not ([digit] || .[digit])
	if (!(traits::isdigit(*str) || (str[0] == CHAR('.') && traits::isdigit(str[1]))))
	{
		if (endp)
			*endp = const_cast<CHAR*>(str);
		error = EINVAL;
		return 0;
	}

	int base = 10;
	int exponent = 0;
	int exponents_per_digit = 1;

	// check whether we have base 16 value -- (0x[xdigit] || 0x.[xdigit])
	if (traits::strncasecmp(str, traits::lit_0x, 2) == 0 && (traits::isxdigit(str[2]) || (str[2] == CHAR('.') && traits::isxdigit(str[3]))))
	{
		base = 16;
		exponents_per_digit = 4;
		str += 2;
	}

	// parse whole part
	T result = 0;
	T multiplier = 1;
	while (true)
	{
		int digit = get_base_digit(*str, base);
		if (digit == -1)
			break;
		str++;

		if (result)
			exponent += exponents_per_digit;
		if (digit)
			result += multiplier * digit;
		if (result)
			multiplier /= base;
	}

	if (*str == CHAR('.'))
		str++;

	while (true)
	{
		const int digit = get_base_digit(*str, base);
		if (digit == -1)
			break;
		str++;

		if (result == 0)
			exponent -= exponents_per_digit;
		if (digit)
			result += multiplier * digit;
		if (result)
			multiplier /= base;
	}

	if (traits::tolower(*str) == (base == 10 ? CHAR('e') : CHAR('p')))
	{
		CHAR* maybe_end = nullptr;
		int exp_error = 0;

		const int extra_exponent = strtoT<int>(str + 1, &maybe_end, 10, exp_error);
		if (exp_error != EINVAL)
		{
			if (exp_error == ERANGE || BAN::Math::will_addition_overflow(exponent, extra_exponent))
				exponent = negative ? BAN::numeric_limits<int>::min() : BAN::numeric_limits<int>::max();
			else
				exponent += extra_exponent;
			str = maybe_end;
		}
	}

	if (endp)
		*endp = const_cast<CHAR*>(str);

	// no over/underflow can happed with zero
	if (result == 0)
		return 0;

	const int max_exponent = (base == 10) ? BAN::numeric_limits<T>::max_exponent10() : BAN::numeric_limits<T>::max_exponent2();
	if (exponent > max_exponent)
	{
		error = ERANGE;
		result = BAN::numeric_limits<T>::infinity();
		return negative ? -result : result;
	}

	const int min_exponent = (base == 10) ? BAN::numeric_limits<T>::min_exponent10() : BAN::numeric_limits<T>::min_exponent2();
	if (exponent < min_exponent)
	{
		error = ERANGE;
		result = 0;
		return negative ? -result : result;
	}

	if (exponent)
		result *= BAN::Math::pow<T>((base == 10) ? 10 : 2, exponent);
	return negative ? -result : result;
}

#endif
