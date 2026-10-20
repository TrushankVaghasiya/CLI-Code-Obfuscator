# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-src")
  file(MAKE_DIRECTORY "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-src")
endif()
file(MAKE_DIRECTORY
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-build"
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix"
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/tmp"
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/src/tree_sitter_cpp-populate-stamp"
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/src"
  "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/src/tree_sitter_cpp-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/src/tree_sitter_cpp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/TIRTH/OOPS Project/CLI-Code-Obfuscator/_b/_deps/tree_sitter_cpp-subbuild/tree_sitter_cpp-populate-prefix/src/tree_sitter_cpp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
