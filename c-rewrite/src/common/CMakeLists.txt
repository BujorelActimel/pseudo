# Common utilities library
# Will be implemented in later tasks

# Placeholder for now
add_library(pseudo_common INTERFACE)
target_include_directories(pseudo_common INTERFACE ${CMAKE_SOURCE_DIR}/include)

# TODO: Add source files as they are created:
# add_library(pseudo_common
#     hashmap.c
#     arena.c
#     vector.c
#     utf8.c
#     error.c
# )
