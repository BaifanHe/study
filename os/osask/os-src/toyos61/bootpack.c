/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include "common.h"
#include "boot.h"
#include "graphic.h"
#include "desc_tbl.h"
#include "pic.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "layer.h"
#include "timer.h"


extern fifo8_t g_keybuf;
extern fifo8_t g_mousebuf;
extern timer_ctrl_t g_timerctl;



static void 
drawstring_and_refresh(layer_t* layer, int x, int y, 
    int color, int bg_color, char* string, int len)
{
  /*
   * x, y => position of string 
   * color => color of string
   * bg_color => background color of string 
   * string => string to display
   * len => length of string
   */

  fill_box8(layer->buf, layer->w_size, bg_color, 
      x, y, x + len * 8 - 1, y + 15);
  draw_font8_asc(layer->buf, layer->w_size, x, y, color, string);
  layers_refresh(layer, x, y, x + len * 8, y + 16);
}

static void 
make_window8(unsigned char* buf, int width, int height, char* title)
{
  static char s_close_btn[14][16] = {
    "OOOOOOOOOOOOOOO@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQ@@QQQQ@@QQ$@", 
    "OQQQQ@@QQ@@QQQ$@", 
    "OQQQQQ@@@@QQQQ$@", 
    "OQQQQQQ@@QQQQQ$@", 
    "OQQQQQ@@@@QQQQ$@", 
    "OQQQQ@@QQ@@QQQ$@", 
    "OQQQ@@QQQQ@@QQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "OQQQQQQQQQQQQQ$@", 
    "O$$$$$$$$$$$$$$@", 
    "@@@@@@@@@@@@@@@@", 
  };
  int x, y;
  char c;

  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, width - 1, 0);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, width - 2, 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 0, 0, 0, height - 1);
  fill_box8(buf, width, COLOR8_FFFFFF, 1, 1, 1, height - 2);
  fill_box8(buf, width, COLOR8_848484, width - 2, 1, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, width - 1, 0, width - 1, height - 1);
  fill_box8(buf, width, COLOR8_C6C6C6, 2, 2, width - 3, height - 3);
  fill_box8(buf, width, COLOR8_000084, 3, 3, width - 4, 20);
  fill_box8(buf, width, COLOR8_848484, 
      1, height - 2, width - 2, height - 2);
  fill_box8(buf, width, COLOR8_000000, 
      0, height - 1, width - 1, height - 1);
  draw_font8_asc(buf, width, 24, 4, COLOR8_FFFFFF, title);

  for (y = 0; y < 14; ++y) {
    for (x = 0; x < 16; ++x) {
      c = s_close_btn[y][x];
      switch (c) {
      case '@':
        c = COLOR8_000000; break;
      case '$':
        c = COLOR8_848484; break;
      case 'Q':
        c = COLOR8_C6C6C6; break;
      default:
        c = COLOR8_FFFFFF;
      }
      buf[(5 + y) * width + (width - 21 + x)] = c;
    }
  }
}



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  fifo8_t timerfifo1, timerfifo2, timerfifo3;
  char debug_info[64], keybuf[32], mousebuf[128]; 
  char timerbuf1[8], timerbuf2[8], timerbuf3[8];
  timer_t* timer1;
  timer_t* timer2;
  timer_t* timer3;
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;
  layer_mgr_t* layer_mgr;
  layer_t* back_layer;
  layer_t* mouse_layer;
  layer_t* win_layer;
  unsigned char* back_buf;
  unsigned char  mouse_buf[256];
  unsigned char* win_buf;

  init_gdt_idt();
  init_pic();
  io_sti();   /* after initialize IDT/PIC, allow all CPU's interruptors */

  fifo_init(&g_keybuf, keybuf, 32);
  fifo_init(&g_mousebuf, mousebuf, 128);
  init_pit(); /* initialize programmable interval timer */
  io_out8(PIC0_IMR, 0xf8);  /* set PIT/PIC1/keyboard permission 11111000 */
  io_out8(PIC1_IMR, 0xef);  /* set mouse permission 11101111 */

  fifo_init(&timerfifo1, timerbuf1, 8);
  timer1 = timer_alloc();
  timer_init(timer1, &timerfifo1, 1);
  timer_settimer(timer1, 1000);

  fifo_init(&timerfifo2, timerbuf2, 8);
  timer2 = timer_alloc();
  timer_init(timer2, &timerfifo2, 1);
  timer_settimer(timer2, 300);

  fifo_init(&timerfifo3, timerbuf3, 8);
  timer3 = timer_alloc();
  timer_init(timer3, &timerfifo3, 1);
  timer_settimer(timer3, 50);


  init_keyboard();      /* initialize keyboard */
  enable_mouse(&mdec);  /* enabled mouse */

  memory_total = memory_test(0x00400000, 0xbfffffff);
  mem_mgr_init(mem_mgr);
  mem_mgr_free(mem_mgr, 0x00001000, 0x0009e000);  /*0x00001000~0x0009e000*/
  mem_mgr_free(mem_mgr, 0x00400000, memory_total - 0x00400000);

  init_palette();
  layer_mgr = layer_mgr_init(mem_mgr, binfo->vram, 
      binfo->screen_x, binfo->screen_y);
  back_layer = layer_alloc(layer_mgr);
  mouse_layer = layer_alloc(layer_mgr);
  win_layer = layer_alloc(layer_mgr);
  back_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 
      binfo->screen_x * binfo->screen_y);
  win_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 160 * 52);
  layer_setbuf(back_layer, back_buf, 
      binfo->screen_x, binfo->screen_y, -1);
  layer_setbuf(mouse_layer, mouse_buf, 16, 16, 99);
  layer_setbuf(win_layer, win_buf, 160, 52, -1);
  init_screen(back_buf, binfo->screen_x, binfo->screen_y);
  init_mouse_cursor8(mouse_buf, 99);
  make_window8(win_buf, 160, 52, "Counter");
  layer_slide(back_layer, 0, 0);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;
  layer_slide(mouse_layer, mouse_x, mouse_y);
  layer_slide(win_layer, 80, 72);
  layer_updown(back_layer, 0);
  layer_updown(win_layer, 1);
  layer_updown(mouse_layer, 2);
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  drawstring_and_refresh(back_layer, 0, 0, 
      COLOR8_FFFFFF, COLOR8_848484, debug_info, 10);

  sprintf(debug_info, "memory total: %dMB, free space: %dKB", 
      memory_total / (1024 * 1024), mem_mgr_total(mem_mgr) / 1024);
  drawstring_and_refresh(back_layer, 0, 32, 
      COLOR8_FFFFFF, COLOR8_848484, debug_info, 40);

  for ( ; ; ) {
    sprintf(debug_info, "%010d", g_timerctl.count);
    drawstring_and_refresh(win_layer, 40, 28, 
        COLOR8_000000, COLOR8_C6C6C6, debug_info, 10);

    io_cli();
    if (0 == fifo_size(&g_keybuf) 
        && 0 == fifo_size(&g_mousebuf)
        && 0 == fifo_size(&timerfifo1) 
        && 0 == fifo_size(&timerfifo2)
        && 0 == fifo_size(&timerfifo3)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          drawstring_and_refresh(back_layer, 0, 16, 
              COLOR8_FFFFFF, COLOR8_848484, debug_info, 2);
      }
      else if (0 != fifo_size(&g_mousebuf)) {
        data = fifo_get(&g_mousebuf);
        io_sti();

        if (0 != mouse_decode(&mdec, data)) {
          /* show all mouse bytes code */
          sprintf(debug_info, "[lcr %4d %4d]", mdec.x, mdec.y);
          if (0 != (mdec.state & 0x01)) 
            debug_info[1] = 'L';
          if (0 != (mdec.state & 0x02))
            debug_info[3] = 'R';
          if (0 != (mdec.state & 0x04))
            debug_info[2] = 'C';
          drawstring_and_refresh(back_layer, 32, 16, 
              COLOR8_FFFFFF, COLOR8_848484, debug_info, 15);

          mouse_x += mdec.x;
          mouse_y += mdec.y;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x > binfo->screen_x - 1)
            mouse_x = binfo->screen_x - 1;
          if (mouse_y > binfo->screen_y - 1)
            mouse_y = binfo->screen_y - 1;

          sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
          drawstring_and_refresh(back_layer, 0, 0, 
              COLOR8_FFFFFF, COLOR8_848484, debug_info, 10);
          layer_slide(mouse_layer, mouse_x, mouse_y);
        }
      }
      else if (0 != fifo_size(&timerfifo1)) {
        data = fifo_get(&timerfifo1);
        io_sti();

        drawstring_and_refresh(back_layer, 0, 64, 
            COLOR8_FFFFFF, COLOR8_848484, "10[sec]", 7);
      }
      else if (0 != fifo_size(&timerfifo2)) {
        data = fifo_get(&timerfifo2);
        io_sti();

        drawstring_and_refresh(back_layer, 0, 80, 
            COLOR8_FFFFFF, COLOR8_848484, "03[sec]", 7);
      }
      else if (0 != fifo_size(&timerfifo3)) {
        data = fifo_get(&timerfifo3);
        io_sti();

        if (0 != data) {
          timer_init(timer3, &timerfifo3, 0);
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_FFFFFF, 8, 96, 15, 111);
        }
        else {
          timer_init(timer3, &timerfifo3, 1);
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_848484, 8, 96, 15, 111);
        }

        timer_settimer(timer3, 50);
        layers_refresh(back_layer, 8, 96, 16, 112);
      }
    }
  }
}

