idf_component_register(SRCS "lcd.c"
                       INCLUDE_DIRS "."
                       REQUIRES
                           RoboEyes
                           esp_lcd
                           esp_system
                           driver
                           freertos
                           esp_timer)
