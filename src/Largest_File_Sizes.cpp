/**
 * @file    Largest_File_Sizes.cpp
 * @date    Started Tue 03 Apr 2018 09:05:13 PM AEST
 * @author  Lester J. Dowling
 * @par     Copyright &copy; 2018 Lester J. Dowling.
 */
#include "Largest_File_Sizes.hpp"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "config.hpp"
namespace fs = boost::filesystem;
namespace {

	void warn_filesystem(const fs::filesystem_error& e__)
	{
		using std::cerr;
		using std::endl;
		cerr << "~~~ [" << APPLICATION_NAME << "] Filesystem warning";
		const int err_code{ e__.code().value() };
		cerr << '(' << '#' << err_code << ')';
		const std::string what{ e__.what() };
		if (what.empty()) {
			cerr << '.' << endl;
		} else {
			cerr << ':' << ' ' << what << endl;
		}
	}

	std::string human_file_size(boost::uintmax_t file_size)
	{
		constexpr double KB = 1024;
		constexpr double MB = KB * KB;
		constexpr double GB = KB * MB;
		double hs;
		const char* units;
		if (0 != std::floor(file_size / GB)) {
			hs = file_size / GB;
			units = "GB";
		} else if (0 != std::floor(file_size / MB)) {
			hs = file_size / MB;
			units = "MB";
		} else if (0 != std::floor(file_size / KB)) {
			hs = file_size / KB;
			units = "KB";
		} else {
			hs = file_size;
			units = "B";
		}
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(1) << hs << ' ' << units;
		return oss.str();
	}
}

bool Largest_File_Sizes::contains(const boost::filesystem::path& pn)
{
	for (const auto& ps : mList)
		if (ps._path == pn)
			return true;
	return false;
}

void Largest_File_Sizes::add(const boost::filesystem::path& pn)
{
	if (boost::filesystem::is_regular_file(pn) && !this->contains(pn)) {
		mList.push_back({ pn, boost::filesystem::file_size(pn) });
		std::sort(mList.begin(), mList.end());
		while (K < mList.size())
			mList.pop_back();
	}
}

void Largest_File_Sizes::recurse_through_directory(const boost::filesystem::path& pn)
{
	std::vector<fs::path> directory_queue;
	directory_queue.reserve(1000);
	if (is_regular_file(pn))
		this->add(pn);
	else if (is_directory(pn)) {
		directory_queue.push_back(pn);
		while (!directory_queue.empty()) {
			const fs::path dir_pn = directory_queue.back();
			directory_queue.pop_back();
			try {
				for (const auto& x : fs::directory_iterator(dir_pn)) {
					const fs::path entry_pn = fs::canonical(x.path());
					if (is_regular_file(entry_pn))
						this->add(entry_pn);
					else if (is_directory(entry_pn)) {
						directory_queue.push_back(entry_pn);
					}
				}
			} catch (const boost::filesystem::filesystem_error& e__) {
				if (e__.code() == std::errc::too_many_symbolic_link_levels)
					throw; // TODO: What should be done here?
				if (e__.code() == std::errc::permission_denied) // ignored
					return;
				if (e__.code() == std::errc::no_such_file_or_directory) // ignored
					return;
				warn_filesystem(e__);
			}
		}
	}
}

void Largest_File_Sizes::print_list(std::ostream& os)
{
	using std::endl;
	using std::setw;
	std::reverse(mList.begin(), mList.end());
	for (const auto& ps : mList) {
		os << setw(11) << human_file_size(ps._size) << '\t' << ps._path.string() << endl;
	}
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

BOOST_AUTO_TEST_SUITE(Largest_File_Sizes_cpp);

BOOST_AUTO_TEST_CASE(test_human_file_size_B)
{
	const std::string hfs = human_file_size(1);
	const std::string expected = "1.0 B";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_KB)
{
	const std::string hfs = human_file_size(1024);
	const std::string expected = "1.0 KB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_9_9_KB)
{
	const std::string hfs = human_file_size(9.9 * 1024);
	const std::string expected = "9.9 KB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_MB)
{
	const std::string hfs = human_file_size(1024 * 1024);
	const std::string expected = "1.0 MB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_2_5_MB)
{
	const std::string hfs = human_file_size(2.5 * 1024 * 1024);
	const std::string expected = "2.5 MB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_GB)
{
	const std::string hfs = human_file_size(1024 * 1024 * 1024);
	const std::string expected = "1.0 GB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_human_file_size_9_99_GB)
{
	const std::string hfs = human_file_size(9.99 * 1024 * 1024 * 1024);
	const std::string expected = "10.0 GB";
	BOOST_CHECK_EQUAL(hfs, expected);
}

BOOST_AUTO_TEST_CASE(test_add_contains)
{
	const fs::path p1{ "/var/log/Xorg.0.log" };
	const fs::path p2{ "/var/log/auth.log" };
	const fs::path p3{ "/var/log/kern.log" };
	const fs::path p4{ "/var/log/syslog" };
	Largest_File_Sizes lfs;
	lfs.add(p1);
	BOOST_CHECK(lfs.contains(p1));
	lfs.add(p2);
	BOOST_CHECK(lfs.contains(p2));
	lfs.add(p3);
	BOOST_CHECK(lfs.contains(p3));
	lfs.add(p4);
	BOOST_CHECK(lfs.contains(p4));
}

BOOST_AUTO_TEST_SUITE_END();
#endif // RUN_UNIT_TESTS
