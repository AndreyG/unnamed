add_executable(int int.cpp)
target_link_libraries(int ${PROJECT_NAME})
set_property(TARGET int PROPERTY CXX_STANDARD 14)

add_executable(file file.cpp)
target_link_libraries(file ${PROJECT_NAME})
set_property(TARGET file PROPERTY CXX_STANDARD 14)

add_executable(polymorphic polymorphic.cpp)
target_link_libraries(polymorphic ${PROJECT_NAME})
set_property(TARGET polymorphic PROPERTY CXX_STANDARD 14)

add_executable(self-destroyer self_destroyer.cpp)
target_link_libraries(self-destroyer ${PROJECT_NAME})
set_property(TARGET self-destroyer PROPERTY CXX_STANDARD 14)

add_executable(incomplete-class
  incomplete_class.cpp
  incomplete_class.h
  incomplete_class_user.cpp
)
target_link_libraries(incomplete-class ${PROJECT_NAME})
set_property(TARGET incomplete-class PROPERTY CXX_STANDARD 14)
