#!/bin/bash
set -e  # Exit whenever a command returns non-zero result
set -x # Trace execution -- for development only.
#set -n # Check syntax only -- for development only.

# @file        open-github-repo.sh
# @author      Lester J. Dowling
# @date        Started 2018-04-04-15-40-10
# @brief       Tell Firefox to open the GitHub page of this repo.

REPO='https://github.com/Lester-Dowling/find-largest-files'

firefox "${REPO}" &
