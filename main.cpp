#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <boost/filesystem.hpp>
#include "config.hpp"
#include "Largest_File_Sizes.hpp"

namespace {
	namespace fs = boost::filesystem;

	void report_exception(const char* title, const std::string& what = std::string{})
	{
		using std::cerr;
		using std::endl;
		cerr << "!!! [" << APPLICATION_NAME << "] " << title;
		if (what.empty()) {
			cerr << '.' << endl;
		} else {
			cerr << ':' << endl;
			cerr << "!!! " << what << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	try {
		if (argc < 2) {
			std::cout << "Usage: find-largest-files directory-path" << std::endl;
			return EXIT_FAILURE;
		}
		Largest_File_Sizes results;
		for (size_t idx = 1; idx < (size_t)argc; ++idx) {
			results.recurse_through_directory(fs::canonical(argv[idx]));
		}
		results.print_list();

		return EXIT_SUCCESS;
	} catch (const boost::filesystem::filesystem_error& e__) {
		report_exception("Filesystem error", e__.what());
	} catch (const std::runtime_error& e__) {
		report_exception("Runtime error", e__.what());
	} catch (const std::exception& e__) {
		report_exception("Exception", e__.what());
	} catch (...) {
		report_exception("Unexpected exception");
	}
	return EXIT_FAILURE;
}
