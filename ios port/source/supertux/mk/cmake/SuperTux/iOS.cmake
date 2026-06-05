set(SUPERTUX_IOS ON CACHE BOOL "Build SuperTux for iOS" FORCE)

if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET OR CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum iOS version" FORCE)
endif()

set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" CACHE STRING
    "Build for iPhone and iPad" FORCE)
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO" CACHE STRING "" FORCE)

set(SUPERTUX_IOS_DEVELOPMENT_TEAM "" CACHE STRING
    "Apple development team ID used for signed iOS device builds")

# Simulator builds do not need signing. Device signing is applied only to the
# final app bundle target, otherwise CMake compiler-detection projects can fail.
if(NOT CMAKE_OSX_SYSROOT MATCHES "iphoneos")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO" CACHE STRING "" FORCE)
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO" CACHE STRING "" FORCE)
endif()
