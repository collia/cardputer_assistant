# CMake configuration for the RoboEyes component

idf_component_register(SRCS "src/FluxGarage_RoboEyes.c"
                       INCLUDE_DIRS "src"
                       REQUIRES lvgl)
