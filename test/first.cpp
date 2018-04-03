/**
 * @file    first.cpp
 * @date    Started 2018-04-03-21-48-24
 */
#define BOOST_TEST_MODULE Unit Tests of find largest files
#include <boost/test/unit_test.hpp>
#include <cmath>
#include "config.hpp"

BOOST_AUTO_TEST_CASE(verify_version)
{
	BOOST_TEST(PROJECT_VERSION_MAJOR == 0);
	BOOST_TEST(PROJECT_VERSION_MINOR == 0);
}

BOOST_AUTO_TEST_CASE(first_arithmetic_test)
{
	double yy = 16.0;
	BOOST_CHECK_EQUAL(sqrt(yy), sqrt(sqrt(yy) * sqrt(yy)));
}
