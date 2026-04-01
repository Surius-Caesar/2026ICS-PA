#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  
  /* Added fields for watchpoint */
  bool used;                // Whether this WP is active
  uint32_t old_val;         // Last evaluated value of the expression
  char expr[128];           // Expression string of this watchpoint
} WP;

#endif
