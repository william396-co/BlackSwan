#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

set(CMAKE_IMPORT_FILE_VERSION 1)

set_property(TARGET protobuf::libprotobuf-lite APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::libprotobuf-lite PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libprotobuf-lited.lib"
)
list(APPEND _cmake_import_check_targets protobuf::libprotobuf-lite)
list(APPEND _cmake_import_check_files_for_protobuf::libprotobuf-lite "${_IMPORT_PREFIX}/lib/debug/libprotobuf-lited.lib")

set_property(TARGET protobuf::libprotobuf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::libprotobuf PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libprotobufd.lib"
)
list(APPEND _cmake_import_check_targets protobuf::libprotobuf)
list(APPEND _cmake_import_check_files_for_protobuf::libprotobuf "${_IMPORT_PREFIX}/lib/debug/libprotobufd.lib")

set_property(TARGET protobuf::libprotoc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::libprotoc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libprotocd.lib"
)
list(APPEND _cmake_import_check_targets protobuf::libprotoc)
list(APPEND _cmake_import_check_files_for_protobuf::libprotoc "${_IMPORT_PREFIX}/lib/debug/libprotocd.lib")

set_property(TARGET protobuf::libupb APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::libupb PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/debug/libupbd.lib"
)
list(APPEND _cmake_import_check_targets protobuf::libupb)
list(APPEND _cmake_import_check_files_for_protobuf::libupb "${_IMPORT_PREFIX}/lib/debug/libupbd.lib")

set_property(TARGET protobuf::protoc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::protoc PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/debug/protoc.exe"
)
list(APPEND _cmake_import_check_targets protobuf::protoc)
list(APPEND _cmake_import_check_files_for_protobuf::protoc "${_IMPORT_PREFIX}/bin/debug/protoc.exe")

set_property(TARGET protobuf::protoc-gen-upb APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::protoc-gen-upb PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upb.exe"
)
list(APPEND _cmake_import_check_targets protobuf::protoc-gen-upb)
list(APPEND _cmake_import_check_files_for_protobuf::protoc-gen-upb "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upb.exe")

set_property(TARGET protobuf::protoc-gen-upbdefs APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::protoc-gen-upbdefs PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upbdefs.exe"
)
list(APPEND _cmake_import_check_targets protobuf::protoc-gen-upbdefs)
list(APPEND _cmake_import_check_files_for_protobuf::protoc-gen-upbdefs "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upbdefs.exe")

set_property(TARGET protobuf::protoc-gen-upb_minitable APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(protobuf::protoc-gen-upb_minitable PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upb_minitable.exe"
)
list(APPEND _cmake_import_check_targets protobuf::protoc-gen-upb_minitable)
list(APPEND _cmake_import_check_files_for_protobuf::protoc-gen-upb_minitable "${_IMPORT_PREFIX}/bin/debug/protoc-gen-upb_minitable.exe")

set(CMAKE_IMPORT_FILE_VERSION)
