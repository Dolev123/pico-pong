# Finds all source files in the current directory
# and save the name to the DIR_PONG variable
aux_source_directory(. DIR_PONG)

include_directories(../lib/Config)
include_directories(../lib/LCD)
include_directories(../lib/Infrared)
include_directories(../lib/GUI)

# Generate the link library
add_library(pong ${DIR_PONG})
target_link_libraries(pong PUBLIC GUI Infrared LCD Config)
