set(Boost_USE_STATIC_LIBS ON)

if (MSVC)
    find_package(Boost REQUIRED stacktrace_windbg)
    list(APPEND copyq_static_LIBRARIES ${Boost_LIBRARIES})
else ()
    find_package(Boost REQUIRED)
    list(APPEND copyq_LIBRARIES dl)
endif ()

include_directories(${Boost_INCLUDE_DIRS})
