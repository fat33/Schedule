find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

add_executable(tst_schedulemodel tst_schedulemodel.cpp)
target_link_libraries(tst_schedulemodel PRIVATE
    schedule_core
    schedule_options
    Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME tst_schedulemodel COMMAND tst_schedulemodel)
# Tests do not need a display.
set_tests_properties(tst_schedulemodel PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
