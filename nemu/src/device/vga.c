#include "common.h"

#ifdef HAS_IOE

#include "device/mmio.h"
#include <SDL2/SDL.h>

#define VMEM 0x40000

#define SCREEN_H 200
#define SCREEN_W 320

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static uint32_t (*vmem) [SCREEN_W];

void vga_vmem_io_handler(paddr_t addr, int len, bool is_write) {
  // Mark screen for update on every write to VRAM
  if (is_write) {
    static int call_count = 0;
    if (call_count < 10) {
      printf("[VGA] Write at addr=0x%x, len=%d\n", addr, len);
      call_count++;
    }
    mark_screen_update();
  }
}

void update_screen() {
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(vmem[0][0]));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void init_vga() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(SCREEN_W * 2, SCREEN_H * 2, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "NEMU");
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);

  vmem = add_mmio_map(VMEM, 0x80000, vga_vmem_io_handler);
  
  // Clear VRAM to black initially
  memset(vmem, 0, SCREEN_W * SCREEN_H * sizeof(uint32_t));
  
  // Force initial screen update
  update_screen();
}
#endif	/* HAS_IOE */
