import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  integrations: [
    starlight({
      title: 'STM32F103C8T6 速查',
      description: 'STM32F103C8T6 寄存器、标准外设库（SPL）与题库',
      locales: { root: { label: '简体中文', lang: 'zh-CN' } },
      customCss: ['./src/styles/site.css'],
      pagination: false,
      sidebar: [
        { label: '外设速查', items: [
          { label: 'GPIO 寄存器与标准库', slug: 'reference/gpio' },
          { label: 'RCC、AFIO、EXTI 与内核', slug: 'reference/clock-exti' },
          { label: '定时器与 USART', slug: 'reference/tim-usart' },
          { label: 'ADC 与 DMA', slug: 'reference/adc-dma' },
          { label: 'I²C 与 SPI', slug: 'reference/i2c-spi' },
          { label: 'RTC、备份域、电源与看门狗', slug: 'reference/power-time' },
          { label: 'Flash、启动与 CRC', slug: 'reference/memory-crc' },
          { label: 'USB FS Device 与 bxCAN', slug: 'reference/usb-can' },
        ] },
        { label: '题库', slug: 'questions' },
        { label: '芯片覆盖清单', slug: 'coverage' },
      ],
    }),
  ],
});
