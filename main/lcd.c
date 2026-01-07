#include "FluxGarage_RoboEyes.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "lvgl/lvgl.h"

static esp_lcd_panel_handle_t g_lcd = NULL;
static lv_display_t *g_disp = NULL;

// lv_obj_t *label;
static lv_obj_t *canvas;
static lv_color_t *canvas_buf;

volatile bool lcd_transfer_in_progress = false;
static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io,
                                esp_lcd_panel_io_event_data_t *event_data,
                                void *user_ctx) {
  lcd_transfer_in_progress = false;
  if (g_disp) {
    lv_display_flush_ready(g_disp);
  }
  return false;
}
// LCD
#define LCD_PIXEL_CLOCK_HZ (80 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LCD_HOST SPI2_HOST

#define LCD_SCREEN_WIDTH 240
#define LCD_SCREEN_HEIGHT 135

// Robo eyes primitives
static lv_obj_t *robo_canvas;
static lv_color_t *robo_buf;

void robo_canvas_init(void) {
  int w = LCD_SCREEN_WIDTH;
  int h = LCD_SCREEN_HEIGHT;

  robo_buf = heap_caps_malloc(w * h * sizeof(lv_color_t),
                              MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
  assert(robo_buf);

  robo_canvas = lv_canvas_create(lv_scr_act());
  lv_canvas_set_buffer(robo_canvas, robo_buf, w, h, LV_COLOR_FORMAT_NATIVE);

  lv_obj_center(robo_canvas);
}

static void clearDisplay(void) {
  lv_canvas_fill_bg(robo_canvas, lv_color_black(), LV_OPA_COVER);
}

static void updateDisplay(void) { lv_timer_handler(); }

static void drawRoundedRectangle(int x, int y, int w, int h, int r,
                                 uint8_t color) {
  lv_layer_t layer;
  printf("Rect %d %d %d %d %hhx\n", x, y, w, h, color);
  lv_canvas_init_layer(robo_canvas, &layer);

  lv_draw_rect_dsc_t dsc;
  lv_draw_rect_dsc_init(&dsc);

  //dsc.bg_color.blue = color + 100;
  if (color) {
      dsc.bg_color.blue = 100;
      dsc.bg_color.red = 0;
      dsc.bg_color.green = 0;
  } else {
      dsc.bg_color.blue = 0;
      dsc.bg_color.red = 0;
      dsc.bg_color.green = 0;
  }
  dsc.bg_opa = LV_OPA_COVER;
  dsc.radius = r;

  lv_area_t a = {.x1 = x, .y1 = y, .x2 = x + w - 1, .y2 = y + h - 1};

  lv_draw_rect(&layer, &dsc, &a);

  lv_canvas_finish_layer(robo_canvas, &layer);
}

static void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2,
                         uint8_t color) {
#if 1
  lv_layer_t layer;
  lv_canvas_init_layer(robo_canvas, &layer);
  printf("tria %d %d %d %d %d %d %hhx\n", x0, y0, x1, y1, x2, y2, color);

  lv_draw_triangle_dsc_t dsc;
  lv_draw_triangle_dsc_init(&dsc);
  if (color) {
      dsc.color.blue = 100;
      dsc.color.red = 0;
      dsc.color.green = 0;
  } else {
      dsc.color.blue = 0;
      dsc.color.red = 0;
      dsc.color.green = 0;
  }
  dsc.opa = LV_OPA_COVER;

  dsc.p[0].x = x0;
  dsc.p[0].y = y0;
  dsc.p[1].x = x1;
  dsc.p[1].y = y1;
  dsc.p[2].x = x2;
  dsc.p[2].y = y2;
  lv_draw_triangle(&layer, &dsc);

  lv_canvas_finish_layer(robo_canvas, &layer);

#else
  lv_layer_t layer;
  lv_canvas_init_layer(robo_canvas, &layer);
  printf("tria %d %d %d %d %d %d %hhx\n", x0, y0, x1, y1, x2, y2, color);

  lv_draw_line_dsc_t dsc;
  lv_draw_line_dsc_init(&dsc);

  dsc.color.blue = color + 100;

  dsc.width = 10;

  /* Line 1 */
  dsc.p1.x = x0;
  dsc.p1.y = y0;
  dsc.p2.x = x1;
  dsc.p2.y = y1;
  lv_draw_line(&layer, &dsc);

  /* Line 2 */
  dsc.p1.x = x1;
  dsc.p1.y = y1;
  dsc.p2.x = x2;
  dsc.p2.y = y2;
  lv_draw_line(&layer, &dsc);

  /* Line 3 */
  dsc.p1.x = x2;
  dsc.p1.y = y2;
  dsc.p2.x = x0;
  dsc.p2.y = y0;
  lv_draw_line(&layer, &dsc);

  lv_canvas_finish_layer(robo_canvas, &layer);
#endif
}

static uint32_t millis() {
  // static int i = 100;
  //  Implementation for getting the current time in milliseconds
  return esp_timer_get_time() / 1000;
  // return i += 100;
}

static uint32_t robo_eyes_random(uint32_t limit) {
  // Implementation for generating a random number
  uint32_t r = esp_random();
  uint32_t d = UINT32_MAX / limit;
  return r / d;
}
#define LCD_MOSI 35
#define LCD_SCLK 36
#define LCD_CS 37
#define LCD_DC 34
#define LCD_RST 33
#define LCD_BLK 38
#define LCD_MISO -1

esp_lcd_panel_handle_t setup_lcd_spi() {

  esp_lcd_panel_handle_t spi_lcd_handle = NULL;
  esp_lcd_panel_io_handle_t io_handle = NULL;

  gpio_config_t bk_gpio_config = {
      .pin_bit_mask = 1ULL << LCD_BLK,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
  gpio_set_level(LCD_BLK, 1);

  spi_bus_config_t buscfg = {
      .sclk_io_num = LCD_SCLK,
      .mosi_io_num = LCD_MOSI,
      .miso_io_num = LCD_MISO,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = LCD_SCREEN_WIDTH * 80 * sizeof(uint16_t),
  };
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

  esp_lcd_panel_io_spi_config_t io_config = {.dc_gpio_num = LCD_DC,
                                             .cs_gpio_num = LCD_CS,
                                             .pclk_hz = LCD_PIXEL_CLOCK_HZ,
                                             .lcd_cmd_bits = LCD_CMD_BITS,
                                             .lcd_param_bits = LCD_PARAM_BITS,
                                             .spi_mode = 0,
                                             .trans_queue_depth = 10,
                                             .on_color_trans_done =
                                                 on_color_trans_done};
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                           &io_config, &io_handle));

  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = LCD_RST,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
      .bits_per_pixel = 16,
  };

  ESP_ERROR_CHECK(
      esp_lcd_new_panel_st7789(io_handle, &panel_config, &spi_lcd_handle));

  esp_lcd_panel_set_gap(spi_lcd_handle, 40, 53);
  ESP_ERROR_CHECK(esp_lcd_panel_reset(spi_lcd_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(spi_lcd_handle));
  esp_lcd_panel_swap_xy(spi_lcd_handle, true);
  esp_lcd_panel_mirror(spi_lcd_handle, true, false);
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(spi_lcd_handle, true));
  esp_lcd_panel_invert_color(spi_lcd_handle, true);

  return spi_lcd_handle;
}

static void lcd_init(void) { g_lcd = setup_lcd_spi(); }

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area,
                          uint8_t *px_map) {
  int x1 = area->x1;
  int y1 = area->y1;
  int x2 = area->x2 + 1;
  int y2 = area->y2 + 1;

  esp_lcd_panel_draw_bitmap(g_lcd, x1, y1, x2, y2, px_map);

  // lv_display_flush_ready(disp);
}

static void lv_tick_task(void *arg) { lv_tick_inc(1); }

void lvgl_task(void *arg) {
  while (1) {
    RoboEyes_update();

    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void blink_task(void *arg) {
  // uint32_t counter = 0;
  // char buffer [10];
  uint32_t i = 0;
  while (1) {
    // snprintf(buffer, sizeof(buffer), "idx %"PRIu32, counter);
    // lv_label_set_text(label, buffer);
    //RoboEyes_update();
    // counter ++;

    switch(i) {

        case 1:
            RoboEyes_anim_confused();
        break;
        case 2:
            RoboEyes_anim_laugh();
        break;
        case 3:
            RoboEyes_setMood(TIRED);
        break;
        case 4:
            RoboEyes_setMood(ANGRY);
        break;
        case 5:
            RoboEyes_setMood(HAPPY);
        break;
        default:
            RoboEyes_setMood(DEFAULT);
            i = 0;
    }
    i ++;

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

static void lvgl_tick_init() {
  esp_timer_handle_t tick_timer;
  const esp_timer_create_args_t tick_args = {.callback = &lv_tick_task,
                                             .name = "lv_tick"};
  esp_timer_create(&tick_args, &tick_timer);
  esp_timer_start_periodic(tick_timer, 1000);
}
static void lvgl_display_init(void) {
  // static lv_display_t *disp;
  static lv_draw_buf_t draw_buf;
  static void *buf1;

  const uint32_t buf_height = 40;
  const uint32_t stride = LCD_SCREEN_WIDTH * 2; // RGB565
  const uint32_t buf_size = stride * buf_height;

  buf1 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  assert(buf1);

  lv_draw_buf_init(&draw_buf, LCD_SCREEN_WIDTH, buf_height,
                   LV_COLOR_FORMAT_RGB565, stride, buf1, buf_size);

  g_disp = lv_display_create(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);

  /* 🔴 REQUIRED IN LVGL v9 */
  lv_display_set_color_format(g_disp, LV_COLOR_FORMAT_RGB565);

  lv_display_set_draw_buffers(g_disp, &draw_buf, NULL);
  lv_display_set_flush_cb(g_disp, lvgl_flush_cb);
}

void app_main(void) {
  lcd_init(); // Your ST7789 init
  lv_init();  // LVGL core

  lvgl_display_init(); // Your flush_cb + buffers

  lvgl_tick_init(); // esp_timer (1 ms)

  xTaskCreatePinnedToCore(lvgl_task, "lvgl", 4096, NULL, 5, NULL, 0);

  // Initialize RoboEyes
  RoboEyes_init(drawRoundedRectangle, // Function to draw rounded rectangles
                drawTriangle,         // Function to draw triangles
                clearDisplay,         // Function to clear the display
                updateDisplay,        // Function to update the display
                millis, // Function to get the current time in milliseconds
                robo_eyes_random // Function to generate random numbers
  );
  robo_canvas_init();
  RoboEyes_begin(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT, 100);
  // Define some automated eyes behaviour
  RoboEyes_setAutoblinker2(ON, 3, 2);
  RoboEyes_setIdleMode2(ON, 2, 2);
  // RoboEyes_setCyclops(ON);
  //  label = lv_label_create(lv_screen_active());
  //  lv_label_set_text(label, "Hello Cardputer!");
  //  lv_obj_center(label);

  xTaskCreatePinnedToCore(blink_task, "blink", 4096, NULL, 5, NULL, 0);
}
