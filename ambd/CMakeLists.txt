add_executable(ambd core.cpp main.cpp pluginloader.cpp)

include_directories(${include_dirs} )
target_link_libraries(ambd ${link_libraries} amb)

install (TARGETS ambd RUNTIME DESTINATION bin)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION /etc/ambd)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/ambd.service DESTINATION /usr/lib/systemd/system)

