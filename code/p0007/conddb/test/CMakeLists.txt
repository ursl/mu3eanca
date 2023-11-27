#

link_libraries(mu3e::conddb)

foreach(test
    cdbInitDB
    cdbPrintPayload
)
    add_executable(${test} ${test}.cpp )
endforeach()
