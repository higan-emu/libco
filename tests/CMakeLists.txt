# Set OS and processor architecture


function(add_co_test name file)
    add_executable(${name} ${file})
    target_link_libraries(${name} libco)
    if (NOT MSVC)
        target_link_libraries(${name} -lc -lm -lnosys)
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()


add_co_test(test_simple test_simple.c)
