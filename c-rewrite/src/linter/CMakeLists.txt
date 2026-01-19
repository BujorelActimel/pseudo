# Linter library
# Will be implemented in TASK-003

# Placeholder for now
add_library(pseudo_linter INTERFACE)
target_include_directories(pseudo_linter INTERFACE ${CMAKE_SOURCE_DIR}/include)

# TODO: Add source files when implementing TASK-003:
# add_library(pseudo_linter
#     linter.c
#     char_map.c
# )
#
# target_link_libraries(pseudo_linter PRIVATE pseudo_common)
