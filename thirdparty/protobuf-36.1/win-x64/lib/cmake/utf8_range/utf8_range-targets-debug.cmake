#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

set(CMAKE_IMPORT_FILE_VERSION 1)

set_property(TARGET utf8_range::utf8_validity APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(utf8_range::utf8_validity PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libutf8_validity.lib"
)
list(APPEND _cmake_import_check_targets utf8_range::utf8_validity)
list(APPEND _cmake_import_check_files_for_utf8_range::utf8_validity "${_IMPORT_PREFIX}/lib/debug/libutf8_validity.lib")

set_property(TARGET utf8_range::utf8_range APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(utf8_range::utf8_range PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libutf8_range.lib"
)
list(APPEND _cmake_import_check_targets utf8_range::utf8_range)
list(APPEND _cmake_import_check_files_for_utf8_range::utf8_range "${_IMPORT_PREFIX}/lib/debug/libutf8_range.lib")

set(CMAKE_IMPORT_FILE_VERSION)
