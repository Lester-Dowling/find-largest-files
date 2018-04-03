#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>
#include "config.hpp"

namespace {
	namespace fs = boost::filesystem;

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
		oss << std::fixed << std::setprecision(1) << std::setw(8) << hs << ' ' << units;
		return oss.str();
	}

	/**
	 * Keep a list of the top k file sizes.
	 */
	class Largest_File_Sizes : private boost::noncopyable
	{
		/**
		 * A private local pair to keep a regular file path and its corresponding
		 * file size.
		 */
		struct path_and_size
		{
			fs::path _path;			// Path of a regular file.
			boost::uintmax_t _size; // Size of the regular file.

			/**
			 * Comparison operator which compares file sizes.  Suitable for
			 * std::sort to sort a vector by file size.
			 */
			bool operator<(const path_and_size& rhs) const { return _size > rhs._size; }
		};

		static constexpr size_t K_default = 10;
		std::vector<path_and_size> mList;
		const size_t K{ K_default };

		/**
		 * Private method to determine if the given path has already been added to
		 * the list.
		 */
		bool contains(const fs::path& pn)
		{
			for (const auto& ps : mList) {
				if (ps._path == pn)
					return true;
			}
			return false;
		}

	  public:
		/**
		 * Add the given path of a regular file to the list.  It may be culled
		 * from the list if its file size is too small.
		 */
		void add(const fs::path& pn)
		{
			if (fs::is_regular_file(pn) && !this->contains(pn)) {
				mList.push_back({ pn, fs::file_size(pn) });
				std::sort(mList.begin(), mList.end());
				while (K < mList.size())
					mList.pop_back();
			}
		}

		/**
		 * Process each file in the given directory and recurse down any found
		 * directories.
		 */
		void recurse_through_directory(const fs::path& pn)
		{
			try {
				if (is_regular_file(pn))
					this->add(pn);
				else if (is_directory(pn))
					for (const auto& x : fs::directory_iterator(pn))
						this->recurse_through_directory(fs::canonical(x.path()));
			} catch (const fs::filesystem_error& e__) {
				if (e__.code() == std::errc::too_many_symbolic_link_levels)
					throw; // TODO: What should be done here?
				if (e__.code() == std::errc::permission_denied) // ignored
					return;
				if (e__.code() == std::errc::no_such_file_or_directory) // ignored
					return;
				warn_filesystem(e__);
			}
		}

		/**
		 * Simply print out the list of discovered large files and their sizes.
		 */
		void print_list()
		{
			using std::cout;
			using std::endl;
			std::reverse(mList.begin(), mList.end());
			for (const auto& ps : mList) {
				cout << human_file_size(ps._size) << '\t' << ps._path.string() << endl;
			}
		}
	};
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
