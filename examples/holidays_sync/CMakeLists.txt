project( holidays_sync )


set(holidays_sync_SRCS holidays.cpp )

add_executable(holidays_sync ${holidays_sync_SRCS})
target_link_libraries(holidays_sync ${QT_QTCORE_LIBRARY} )


