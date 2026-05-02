#include "nemu.h"
#ifdef HAS_IOE
#include "device/mmio.h" //Surius: needed for is_mmio/mmio_read/mmio_write
#endif

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
#ifdef HAS_IOE
  //Surius: Check MMIO first; if mapped to a device, use mmio_read.
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) return mmio_read(addr, len, mmio_id);
#endif
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
#ifdef HAS_IOE
  //Surius: Check MMIO first; if mapped to a device, use mmio_write.
  int mmio_id = is_mmio(addr);
  if (mmio_id != -1) { mmio_write(addr, len, data, mmio_id); return; }
#endif
  memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
