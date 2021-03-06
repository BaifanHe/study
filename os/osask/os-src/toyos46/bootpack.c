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


extern fifo8_t g_keybuf;
extern fifo8_t g_mousebuf;



void 
HariMain(void)
{
  boot_info_t* binfo = (boot_info_t*)ADR_BOOTINFO;
  char debug_info[64], keybuf[32], mousebuf[128];
  int mouse_x, mouse_y;
  int data;
  mouse_dec_t mdec;
  unsigned int memory_total;
  mem_mgr_t* mem_mgr = (mem_mgr_t*)MEMMGR_ADDR;
  layer_mgr_t* layer_mgr;
  layer_t* back_layer;
  layer_t* mouse_layer;
  unsigned char* back_buf;
  unsigned char  mouse_buf[256];

  init_gdt_idt();
  init_pic();
  io_sti();

  fifo_init(&g_keybuf, keybuf, 32);
  fifo_init(&g_mousebuf, mousebuf, 128);
  io_out8(PIC0_IMR, 0xf9);
  io_out8(PIC1_IMR, 0xef);

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
  back_buf = (unsigned char*)mem_mgr_alloc_4k(mem_mgr, 
      binfo->screen_x * binfo->screen_y);
  layer_setbuf(back_layer, back_buf, 
      binfo->screen_x, binfo->screen_y, -1);
  layer_setbuf(mouse_layer, mouse_buf, 16, 16, 99);
  init_screen(back_buf, binfo->screen_x, binfo->screen_y);
  init_mouse_cursor8(mouse_buf, 99);
  layer_slide(layer_mgr, back_layer, 0, 0);
  mouse_x = (binfo->screen_x - 16) / 2;
  mouse_y = (binfo->screen_y - 28 - 16) / 2;
  layer_slide(layer_mgr, mouse_layer, mouse_x, mouse_y);
  layer_updown(layer_mgr, back_layer, 0);
  layer_updown(layer_mgr, mouse_layer, 1);
  sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
  draw_font8_asc(back_buf, binfo->screen_x, 0, 0, 
      COLOR8_FFFFFF, debug_info);

  sprintf(debug_info, "memory total: %dMB, free space: %dKB", 
      memory_total / (1024 * 1024), mem_mgr_total(mem_mgr) / 1024);
  draw_font8_asc(back_buf, binfo->screen_x, 
      0, 32, COLOR8_FFFFFF, debug_info);

  layers_refresh(layer_mgr, back_layer, 0, 0, binfo->screen_x, 48);

  for ( ; ; ) {
    io_cli();

    if (0 == fifo_size(&g_keybuf) && 0 == fifo_size(&g_mousebuf)) 
      io_stihlt();
    else {
      if (0 != fifo_size(&g_keybuf)) {
          data = fifo_get(&g_keybuf);

          io_sti();
          sprintf(debug_info, "%02X", data);
          fill_box8(back_buf, binfo->screen_x, 
            COLOR8_848484, 0, 16, 15, 31);
          draw_font8_asc(back_buf, binfo->screen_x, 
            0, 16, COLOR8_FFFFFF, debug_info);

          layers_refresh(layer_mgr, back_layer, 0, 16, 16, 32);
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
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_848484, 32, 16, 32 + 15 * 8 - 1, 31);
          draw_font8_asc(back_buf, binfo->screen_x, 
              32, 16, COLOR8_FFFFFF, debug_info);
          layers_refresh(layer_mgr, back_layer, 32, 16, 32 + 15 * 8, 32);

          mouse_x += mdec.x;
          mouse_y += mdec.y;

          if (mouse_x < 0)
            mouse_x = 0;
          if (mouse_y < 0)
            mouse_y = 0;
          if (mouse_x > binfo->screen_x - 16)
            mouse_x = binfo->screen_x - 16;
          if (mouse_y > binfo->screen_y - 16)
            mouse_y = binfo->screen_y - 16;

          sprintf(debug_info, "(%3d, %3d)", mouse_x, mouse_y);
          fill_box8(back_buf, binfo->screen_x, 
              COLOR8_848484, 0, 0, 79, 15);     /* hide mouse position */
          draw_font8_asc(back_buf, binfo->screen_x, 
              0, 0, COLOR8_FFFFFF, debug_info); /* show mouse position */

          layers_refresh(layer_mgr, back_layer, 0, 0, 80, 16);
          layer_slide(layer_mgr, mouse_layer, mouse_x, mouse_y);
        }
      }
    }
  }
}

