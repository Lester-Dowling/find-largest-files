/**
 * @file    Largest_File_Sizes.hpp
 * @date    Started Tue 03 Apr 2018 09:05:13 PM AEST
 * @author  Lester J. Dowling
 * @par     Copyright &copy; 2018 Lester J. Dowling.
 */
#pragma once
#include <iostream>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

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
		boost::filesystem::path _path; // Path of a regular file.
		boost::uintmax_t _size;		   // Size of the regular file.

		/**
		 * Comparison operator which compares file sizes.  Suitable for
		 * std::sort to sort a vector by file size.
		 */
		bool operator<(const path_and_size& rhs) const { return _size > rhs._size; }
	};

	static constexpr size_t K_default = 10;
	std::vector<path_and_size> mList;
	const size_t K{ K_default };

  public: // Ctors -------------------------------------------------------------
	Largest_File_Sizes() = default;
	virtual ~Largest_File_Sizes() = default;

  public: // Methods -----------------------------------------------------------
	/**
	 * Add the given path of a regular file to the list.  It may be culled from
	 * the list if its file size is too small.
	 */
	void add(const boost::filesystem::path& pn);

	/**
	 * Determine if the given path has already been added to the list.
	 */
	bool contains(const boost::filesystem::path& pn);

	/**
	 * Process each file in the given directory and recurse down any found
	 * directories.
	 */
	void recurse_through_directory(const boost::filesystem::path& pn);

	/**
	 * Simply print out the list of discovered large files and their sizes.
	 */
	void print_list(std::ostream& os = std::cout);
};
// EOF
