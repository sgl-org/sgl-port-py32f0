/*
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL
 * Document reference link: docs directory
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include "py32f0xx_hal.h"
#include "sgl.h"
#include "sgl_anim.h"
#include "sgl_font.h"

#define TFT_RST_PORT GPIOA
#define TFT_RST_PIN GPIO_PIN_6
#define TFT_DC_PORT GPIOB
#define TFT_DC_PIN GPIO_PIN_1

#define TFT_WIDTH 240
#define TFT_HEIGHT 280

static UART_HandleTypeDef
huart1;

static SPI_HandleTypeDef
hspi1;

static DMA_HandleTypeDef
hdmach1;

static sgl_color_t
panel_buffer[TFT_WIDTH * 2];

static unsigned int
frames;

int
__io_putchar(int ch)
{
    if (ch == '\n')
        HAL_UART_Transmit(&huart1, (void *)"\r", 1, -1);
    HAL_UART_Transmit(&huart1, (void *)&ch, 1, -1);
    return ch;
}

static void
logger(const char *str)
{
    printf("%s\n", str);
}

static void
tft_write_cmd(uint16_t cmd)
{
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, 0);
    HAL_SPI_Transmit(&hspi1, (void *)&cmd, 1, -1);
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, 1);
}

static void
tft_write_data(uint16_t data)
{
    HAL_SPI_Transmit(&hspi1, (void *)&data, 1, -1);
}

static void
tft_set_win(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* Set column address */
    tft_write_cmd(0x2a);
	tft_write_data(x1);
	tft_write_data(x2);

    /* Set row address */
    tft_write_cmd(0x2b);
	tft_write_data(y1 + 20);
	tft_write_data(y2 + 20);

    /* Write frame buffer */
    tft_write_cmd(0x2c);
}

static void
tft_init_sequence(void)
{
	tft_write_cmd(0x11);
	HAL_Delay(120);

    /* Rotation */
	tft_write_cmd(0x36);
	tft_write_data(0x00);

	tft_write_cmd(0x3a);
	tft_write_data(0x05);

    /* Frame rate setting */
	tft_write_cmd(0xb2);
	tft_write_data(0x0c);
	tft_write_data(0x0c);
	tft_write_data(0x00);
	tft_write_data(0x33);
	tft_write_data(0x33);

	tft_write_cmd(0xb7);
	tft_write_data(0x35);

    /* Power setting */
	tft_write_cmd(0xbb);
	tft_write_data(0x32);

    tft_write_cmd(0xc2);
	tft_write_data(0x01);

    tft_write_cmd(0xc3);
	tft_write_data(0x15);

    tft_write_cmd(0xc4);
	tft_write_data(0x20);

    tft_write_cmd(0xc6);
	tft_write_data(0x0f);

	tft_write_cmd(0xd0);
	tft_write_data(0xa4);
	tft_write_data(0xa1);

	tft_write_cmd(0xe0);
	tft_write_data(0xd0);
	tft_write_data(0x08);
	tft_write_data(0x0e);
	tft_write_data(0x09);
	tft_write_data(0x09);
	tft_write_data(0x05);
	tft_write_data(0x31);
	tft_write_data(0x33);
	tft_write_data(0x48);
	tft_write_data(0x17);
	tft_write_data(0x14);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x34);

	tft_write_cmd(0xe1);
	tft_write_data(0xd0);
	tft_write_data(0x08);
	tft_write_data(0x0e);
	tft_write_data(0x09);
	tft_write_data(0x09);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x33);
	tft_write_data(0x48);
	tft_write_data(0x17);
	tft_write_data(0x14);
	tft_write_data(0x15);
	tft_write_data(0x31);
	tft_write_data(0x34);

	tft_write_cmd(0x21);
	HAL_Delay(20);

	tft_write_cmd(0x29);
	HAL_Delay(20);
}

static void
tft_init(void)
{
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, 0);
    HAL_Delay(100);
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, 1);
    HAL_Delay(100);

    tft_init_sequence();
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    HAL_SPI_Init(&hspi1);
}

static void
tft_flush_area(int16_t x, int16_t y, int16_t w, int16_t h, sgl_color_t *src)
{
    while (hspi1.State != HAL_SPI_STATE_READY)
        __NOP();

    tft_set_win(x, y, x + w - 1, y + h - 1);
	HAL_SPI_Transmit_DMA(&hspi1, (void *)src, w * h);
}

static void
demo_anim_path(struct sgl_anim *anim, int32_t value)
{
    sgl_obj_set_pos(anim->obj, value, value);
}

static void
demo_anim_finish(struct sgl_anim *anim)
{
    uint16_t tmp;

    tmp = anim->start_value;
    anim->start_value = anim->end_value;
    anim->end_value = tmp;
}

void
DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(hspi1.hdmatx);
}

void
SysTick_Handler(void)
{
    static unsigned int timer, last;

    if (timer++ >= 1000) {
        printf("Render: %ufps\n", frames - last);
        last = frames;
        timer = 0;
    }

    sgl_anim_tick_inc(1);
    HAL_IncTick();
}

int
main(void)
{
    RCC_OscInitTypeDef OscInitType = {};
    RCC_ClkInitTypeDef ClkInitType = {};
    GPIO_InitTypeDef GPIOInitType = {};

    HAL_Init();

    OscInitType.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    OscInitType.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
    OscInitType.HSIState = RCC_HSI_ON;
    OscInitType.HSIDiv = RCC_HSI_DIV1;
    OscInitType.PLL.PLLState = RCC_PLL_ON;
    OscInitType.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    HAL_RCC_OscConfig(&OscInitType);

    ClkInitType.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;
    ClkInitType.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    ClkInitType.AHBCLKDivider = RCC_SYSCLK_DIV1;
    ClkInitType.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&ClkInitType, FLASH_LATENCY_1);

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_DMA_CLK_ENABLE();
    HAL_SYSCFG_DMA_Req(1);

    /* UART: TX */
    GPIOInitType.Pin = GPIO_PIN_2;
    GPIOInitType.Mode = GPIO_MODE_AF_PP;
    GPIOInitType.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIOInitType.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIOInitType);

    /* TFT: SCK SDA */
    GPIOInitType.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIOInitType.Mode = GPIO_MODE_AF_PP;
    GPIOInitType.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIOInitType.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIOInitType);

    /* TFT: DC */
    GPIOInitType.Pin = TFT_DC_PIN;
    GPIOInitType.Mode = GPIO_MODE_OUTPUT_PP;
    GPIOInitType.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TFT_DC_PORT, &GPIOInitType);

    /* TFT: RST */
    GPIOInitType.Pin = TFT_RST_PIN;
    GPIOInitType.Mode = GPIO_MODE_OUTPUT_PP;
    GPIOInitType.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TFT_RST_PORT, &GPIOInitType);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.Mode = UART_MODE_TX;
    HAL_UART_Init(&huart1);

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
    HAL_SPI_Init(&hspi1);
    tft_init();

    hdmach1.Instance = DMA1_Channel1;
    hdmach1.Init.Mode = DMA_NORMAL;
    hdmach1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdmach1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdmach1.Init.MemInc = DMA_MINC_ENABLE;
    hdmach1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdmach1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdmach1.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdmach1);

    __HAL_LINKDMA(&hspi1, hdmatx, hdmach1);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    sgl_device_fb_t fbdev = {
        .xres = TFT_WIDTH,
        .yres = TFT_HEIGHT,
        .xres_virtual = TFT_WIDTH,
        .yres_virtual = TFT_HEIGHT,
        .flush_area = tft_flush_area,
        .framebuffer = panel_buffer,
        .framebuffer_size = SGL_ARRAY_SIZE(panel_buffer),
    };

    sgl_device_fb_register(&fbdev);
    sgl_device_log_register(logger);
    sgl_init();

    sgl_obj_t *rect1 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect1, 0, 0);
    sgl_obj_set_size(rect1, 50, 50);
    sgl_obj_set_color(rect1, SGL_COLOR_GRAY);
    sgl_obj_set_border_color(rect1, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect1, 3);
    sgl_obj_set_radius(rect1, 10);
    sgl_obj_set_alpha(rect1, 100);

    sgl_obj_t *rect2 = sgl_rect_create(NULL);
    sgl_obj_set_pos(rect2, 0, 0);
    sgl_obj_set_size(rect2, 50, 50);
    sgl_obj_set_color(rect2, SGL_COLOR_BRIGHT_PURPLE);
    sgl_obj_set_border_color(rect2, SGL_COLOR_GREEN);
    sgl_obj_set_border_width(rect2, 3);
    sgl_obj_set_radius(rect2, 10);
    sgl_obj_set_alpha(rect2, 100);

    sgl_anim_t *anim1 = sgl_anim_create();
    sgl_anim_set_obj(anim1, rect1);
    sgl_anim_set_act_duration(anim1, 1200);
    sgl_anim_set_start_value(anim1, 10);
    sgl_anim_set_end_value(anim1, TFT_WIDTH - 50);
    sgl_anim_set_path(anim1, demo_anim_path);
    sgl_anim_set_path_algo(anim1, SGL_ANIM_PATH_LINEAR);
    sgl_anim_set_repeat_cnt(anim1, -1);
    anim1->finish_cb = demo_anim_finish;

    sgl_anim_t *anim2 = sgl_anim_create();
    sgl_anim_set_obj(anim2, rect2);
    sgl_anim_set_act_duration(anim2, 1200);
    sgl_anim_set_start_value(anim2, 50);
    sgl_anim_set_end_value(anim2, TFT_WIDTH - 50);
    sgl_anim_set_path(anim2, demo_anim_path);
    sgl_anim_set_path_algo(anim2, SGL_ANIM_PATH_LINEAR);
    sgl_anim_set_repeat_cnt(anim2, -1);
    anim2->finish_cb = demo_anim_finish;

    sgl_anim_start(anim1);
    sgl_anim_start(anim2);

    for (;;) {
        sgl_task_handle();
        frames++;
    }

    return 0;
}
