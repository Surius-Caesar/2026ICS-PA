#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
    int n = 1; //default

    // if there is a parmeter
    if (args != NULL) {
        sscanf(args, "%d", &n);  //"5"to 5
    }

    cpu_exec(n);  //执行 n 条指令
    return 0;
}

static int cmd_info(char *args) {
    // the error
    if (args == NULL) {
        printf("Usage: info r (register) | info w (watchpoint)\n");
        return 0;
    }

    // r
    if (strcmp(args, "r") == 0) {
        printf("eax: 0x%08x\n", cpu.eax);
        printf("ebx: 0x%08x\n", cpu.ebx);
        printf("ecx: 0x%08x\n", cpu.ecx);
        printf("edx: 0x%08x\n", cpu.edx);
        printf("esp: 0x%08x\n", cpu.esp);
        printf("ebp: 0x%08x\n", cpu.ebp);
        printf("esi: 0x%08x\n", cpu.esi);
        printf("edi: 0x%08x\n", cpu.edi);
        printf("eip: 0x%08x\n", cpu.eip);

	// 16-bit registers
        printf("ax: 0x%04x\n", cpu.gpr[0]._16);
        printf("cx: 0x%04x\n", cpu.gpr[1]._16);
        printf("dx: 0x%04x\n", cpu.gpr[2]._16);
        printf("bx: 0x%04x\n", cpu.gpr[3]._16);
        printf("sp: 0x%04x\n", cpu.gpr[4]._16);
        printf("bp: 0x%04x\n", cpu.gpr[5]._16);
        printf("si: 0x%04x\n", cpu.gpr[6]._16);
        printf("di: 0x%04x\n", cpu.gpr[7]._16);

        // 8-bit low registers
        printf("al: 0x%02x\n", cpu.gpr[0]._8[0]);
        printf("cl: 0x%02x\n", cpu.gpr[1]._8[0]);
        printf("dl: 0x%02x\n", cpu.gpr[2]._8[0]);
        printf("bl: 0x%02x\n", cpu.gpr[3]._8[0]);

        // 8-bit high registers
        printf("ah: 0x%02x\n", cpu.gpr[0]._8[1]);
        printf("ch: 0x%02x\n", cpu.gpr[1]._8[1]);
        printf("dh: 0x%02x\n", cpu.gpr[2]._8[1]);
        printf("bh: 0x%02x\n", cpu.gpr[3]._8[1]);


    }
    // w
    else if (strcmp(args, "w") == 0) {
        wp_display();  // call the watchpoint print function
    }

    return 0;
}

static int cmd_p(char *args) {
    if (args == NULL) {
        printf("Usage: p EXPR\n");
        return 0;
    }
    bool success;
    uint32_t result = expr(args,&success);
    if(!success){
	  printf("Invalid experession!\n");  
	  return 0;
    }
    printf("0x%08x\n", result);  // 16 number system

    return 0;
}

static int cmd_x(char *args) {
    char *n_str = strtok(args, " ");    // get N
    char *addr_expr = strtok(NULL, ""); // get the expression of the adderss

    if (n_str == NULL || addr_expr == NULL) {
	printf("Usage: x N EXPR\n");
        return 0;
    }
	//exception handling
    int n = atoi(n_str);                  // transform N
    bool success;
    uint32_t addr = expr(addr_expr,&success);      

    if(!success){
    printf("Invalid register address!\n");
    return 0;
    }
    
    for (int i = 0; i < n; i++) {
        if (i % 4 == 0) printf("\n0x%08x:", addr + i*4); // output the basic address at the beginning of every line
        uint32_t data = vaddr_read(addr + i*4, 4);  // 4 byte one time
        printf(" 0x%08x", data);
    }
    printf("\n");

    return 0;
}


static int cmd_w(char *args) {
    if (args == NULL) {
        printf("Usage: w EXPR\n");
        return 0;
    };
    // Get a free watchpoint
    WP *wp = new_wp();

    // Mark it as used
    wp->used = true;

    // Save the expression
    strncpy(wp->expr, args, 127);
    wp->expr[127] = '\0';

    // Evaluate the initial value
    bool success;
    uint32_t val = expr(args, &success);
    if (!success) {
        printf("Error: invalid expression\n");
        wp->used = false;
        free_wp(wp);
        return 0;
    }

    wp->old_val = val;

    printf("Watchpoint %d enabled: %s = 0x%08x\n", wp->NO, args, val);
    return 0;
}

static int cmd_d(char *args) {
    if (args == NULL) {
        printf("Usage: d N\n");
        return 0;
    }

    int no = atoi(args);

    WP *p = head;
    while (p != NULL) {
        if (p->NO == no) {
            free_wp(p);
            printf("Watchpoint %d deleted\n", no);
            return 0;
        }
        p = p->next;
    }

    // if not found
    printf("Watchpoint %d not found\n", no);
    return 0;
}


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  // new commands
  { "si", "Single step", cmd_si },
  { "info", "Print information of the program", cmd_info },
  { "p", "Evaluate expression", cmd_p},
  { "x", "Scan the memory", cmd_x},
  { "w", "Set a watchpoint", cmd_w},
  { "d", "Delete a watchpoint",cmd_d},
  {NULL,NULL,NULL}//the symbol of end
   /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
