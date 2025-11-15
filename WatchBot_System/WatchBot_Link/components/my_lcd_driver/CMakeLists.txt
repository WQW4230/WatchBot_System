idf_component_register(
    SRCS "my_lcd_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_lcd_ili9341
)
