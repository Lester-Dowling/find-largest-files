/**
 * @file    expand-user.cpp
 * @date    Started Wed 04 Apr 2018 05:32:14 AM AEST
 * @brief   Copied from
 *          https://stackoverflow.com/questions/4891006/how-to-create-a-folder-in-the-home-directory
 */
#include "expand-user.hpp"
#include <cstddef>
#include <cstdlib>
#include <stdexcept>

extern std::string expand_user(std::string path)
{
	if (not path.empty() and path[0] == '~') {
#if defined(WIN32) || defined(WIN64)
		const char* const HOME = getenv("USERPROFILE");
#else
		const char* const HOME = getenv("HOME");
#endif
		const std::string home{ HOME ? HOME : "" };
		if (!home.empty()) {
			path.replace(0, 1, home);
		} else {
			const char* const HOMEDRIVE = getenv("HOMEDRIVE");
			const char* const HOMEPATH = getenv("HOMEPATH");
			const std::string homedrive{ HOMEDRIVE ? HOMEDRIVE : "" };
			const std::string homepath{ HOMEPATH ? HOMEPATH : "" };
			const std::string homedrivepath{ homedrive + homepath };
			if (!homedrivepath.empty()) {
				path.replace(0, 1, homedrivepath);
			} else
				throw std::invalid_argument{ "No HOME directory defined." };
		}
	}
	return path;
}

// ----------------------------------------------------------------------------
// UNIT TESTS
// ----------------------------------------------------------------------------
#ifdef RUN_UNIT_TESTS
#include <list>
#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

BOOST_AUTO_TEST_SUITE(expand_user_cpp);

static const char* gethome()
{
#if defined(WIN32) || defined(WIN64)
	return getenv("USERPROFILE");
#else
	return getenv("HOME");
#endif
}

BOOST_AUTO_TEST_CASE(test_single_tilde)
{
	const std::string home_path = expand_user("~");
	const std::string expected{ gethome() };
	BOOST_CHECK_EQUAL(home_path, expected);
}

BOOST_AUTO_TEST_CASE(test_canonical)
{
	const boost::filesystem::path home_path{ expand_user("~") };
	const boost::filesystem::path expected{ gethome() };
	BOOST_CHECK_EQUAL(home_path, expected);
}

BOOST_AUTO_TEST_SUITE_END();
#endif // RUN_UNIT_TESTS
