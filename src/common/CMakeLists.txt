add_library(common STATIC
        helper.cpp
)

# Include headers from this directory
target_include_directories(common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)