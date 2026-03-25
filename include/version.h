#ifndef VERSION_H
#define VERSION_H

#define VERSION_MAJOR 1

#define VERSION_MINOR 0

#define VERSION_PATCH 0

#define VERSION_STRINGIFY(x) #x
#define VERSION_TOSTRING(x) VERSION_STRINGIFY(x)

#define VERSION_STRING VERSION_TOSTRING(VERSION_MAJOR) "." \
                       VERSION_TOSTRING(VERSION_MINOR) "." \
                       VERSION_TOSTRING(VERSION_PATCH)

#define VERSION_PACKED ((VERSION_MAJOR << 16) | (VERSION_MINOR << 8) | VERSION_PATCH)

#define LIBRARY_NAME "PCC"
#define LIBRARY_DESCRIPTION "Prompt Context Controller"

#define VERSION_AT_LEAST(major, minor, patch) \
    (VERSION_PACKED >= (((major) << 16) | ((minor) << 8) | (patch)))

#define VERSION_REQUIRE(major, minor, patch) \
    typedef char static_assert_version_##major##_##minor##_##patch[VERSION_AT_LEAST(major, minor, patch) ? 1 : -1]

#endif 
