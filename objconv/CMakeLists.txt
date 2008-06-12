#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
include_directories("..")
SET(TRISORT_SOURCES trisort.cpp)
add_executable(trisort ${TRISORT_SOURCES})

SET(ASTEROID_SOURCES asteroidgen.cpp)
add_executable(asteroidgen ${ASTEROID_SOURCES})

SET(REPLACE_SOURCES replace.cpp)
add_executable(replace ${REPLACE_SOURCES})



