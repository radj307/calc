#pragma once

#include <cmath>

namespace calc {

	// Compound Interest Formulas

	using float_t = long double;
	
	/**
	 * @brief		Calculates the final amount of an account earning compound interest.
	 * @param P	  -	The initial amount in the account.
	 * @param r   -	The annual interest rate as a decimal value.
	 * @param n   -	The number of compunding periods per year. (Annually=1, Semiannually=2, Quarterly=4, Monthly=12)
	 * @param t   -	The number of years.
	 * @returns		The final amount in the account.
	*/
	float_t compundInterest(float_t P, float_t r, float_t n, float_t t)
	{
		return P * pow((1 + r / n), n*t);
	}

	/**
	 * @brief		Calculates the initial amount needed in an account earning compound interest to grow to a desired value.
	 * @param A	  -	The final amount you want in the account.
	 * @param r   -	The annual interest rate as a decimal value.
	 * @param n   -	The number of compunding periods per year. (Annually=1, Semiannually=2, Quarterly=4, Monthly=12) 
	 * @param t   -	The number of years.
	 * @returns		The principal amount needed.
	*/
	float_t compIntPrincipal(float_t A, float_t r, float_t n, float_t t)
	{
		return A / pow((1 + r / n), n * t);
	}

	/**
	 * @brief		Calculates the amount of time needed for an account earning compound interest to grow to a desired value.
	 * @param A	  -	The final amount you want in the account.
	 * @param P	  - The initial amount in the account.
	 * @param r   -	The annual interest rate as a decimal value.
	 * @param n   - The number of compunding periods per year. (Annually=1, Semiannually=2, Quarterly=4, Monthly=12)
	 * @returns		The amount of time needed.
	*/
	float_t compIntTime(float_t A, float_t P, float_t r, float_t n)
	{
		return log10(A / P) / (n * log10(1 + r / n))
	}
}