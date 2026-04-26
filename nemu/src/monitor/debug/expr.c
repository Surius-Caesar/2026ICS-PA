#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#include <stdlib.h>

enum {
  TK_NOTYPE = 256,   // space
  TK_EQ,             // Operator ==
  TK_NE,             // Operator !=
  TK_AND,            // Operator &&
  TK_NUM,            // Decimal number (123)
  TK_HEX,            // Hex number (0x123)
  TK_REG,            // Register name ($eax)
  TK_DEREF           // Pointer dereference (*expr)

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                 // space
  {"0x[0-9a-fA-F]+", TK_HEX},        // Hexadecimal number
  {"[0-9]+", TK_NUM},                // decimal
  {"\\$[a-zA-Z0-9]+", TK_REG},       // register 
  {"\\+", '+'},                      // Plus
  {"-", '-'},                        // Minus / Negation
  {"\\*", '*'},                      // Multiply / Deref
  {"/", '/'},                        // Divide
  {"\\(", '('},                      // Left parenthesis
  {"\\)", ')'},                      // Right parenthesis
  {"==", TK_EQ},                     // Equal
  {"!=", TK_NE},                     // Not equal
  {"&&", TK_AND}                     // Logical and
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
	switch (rules[i].token_type) {
	   // skip space
          case TK_NOTYPE:
            break;
          // number
          case TK_NUM:
            // overflow
            if (nr_token >= 32) {
              printf("too many tokens\n");
              return false;
            }
            tokens[nr_token].type = TK_NUM;
            // 数字字符串，防止溢出
            if (substr_len >= 32) substr_len = 31;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;

	    case TK_HEX:
            case TK_REG:
            // Store hex number or register token
            if (nr_token >= 32) {
              printf("too many tokens\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len >= 32) substr_len = 31;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;

          //  + - * / ( ) ==
          default:
            if (nr_token >= 32) {
              printf("too many tokens\n");
              return false;
            }
            // only store types for others
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  // to distinguish the pointer deference
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*') {
      if (i == 0 ||
          !(tokens[i-1].type == TK_NUM ||
            tokens[i-1].type == TK_HEX ||
            tokens[i-1].type == TK_REG ||
            tokens[i-1].type == ')'))
      {
        tokens[i].type = TK_DEREF;
      }
    }
  }
  return true;
}

// new code


// tool 1
static bool check_parentheses(int p, int q, bool *matched) {
  // presuppose
  *matched = true;
  bool result=true;
  if (tokens[p].type != '(') return false;

  int balance = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') balance++;
    if (tokens[i].type == ')') balance--;

    // negative:unlawful
    if (balance < 0) {
      *matched = false;
      return false;
    }
    // the unexpected(prematually) 0 means the total expression is not wrapped by a pair of paratheses
    if(balance == 0 && i < q) {
    result = false;
    }
  }

  // check the Sum
  if (balance != 0) {
    *matched = false;
    return false;
  }
  //in the non-denial situation,the result is true. 
  return result;
}

// Return operator priority (higher value = higher priority)
static int get_priority(int op) {
  switch (op) {
    case TK_DEREF:  return 4;   // Highest: pointer dereference
    case '*':
    case '/':       return 3;   // Multiply / Divide
    case '+':
    case '-':       return 2;   // Add / Subtract
    case TK_EQ:
    case TK_NE:     return 1;   // Equality check
    case TK_AND:    return 0;   // Lowest: logical AND
    default:        return -1;  // Not an operator
  }
}

// Find the main operator (lowest priority outside parentheses)
static int find_main_op(int p, int q) {
  int main_op_pos = p;
  int min_priority = 100; // presetting
  int balance = 0; // layers of parentheses

  for (int i = p; i <= q; i++) {
    int type = tokens[i].type;

    // Skip operators inside parentheses
    if (type == '(') balance++;
    if (type == ')') balance--;
    if (balance != 0) continue;
    // skip the unary \"-\"
    if (type == '-') {
    bool is_unary = true;
    if (i > p) {  
        int prev = tokens[i-1].type;
        if (prev == TK_NUM || prev == TK_REG || prev == TK_HEX || prev == ')') {
            is_unary = false;
        }
    }
    if (is_unary) continue; 
}


    int prio = get_priority(type);
    if (prio < 0) continue;
    // lower priority means new main op.
    // the same priority means the left should be new main op.
    if (prio <= min_priority) {
      min_priority = prio;
      main_op_pos = i;
    }
  }
  return main_op_pos;
}


static uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    // bad expression
    *success = false;
    return 0;
  }

    // Handle unary negation: -expr
  if (tokens[p].type == '-') {
    bool is_unary = true;
    if (p > 0) {
      int t = tokens[p - 1].type;
      // Not unary if previous token is number/register/')'
      if (t == TK_NUM || t == TK_HEX || t == TK_REG || t == ')')
        is_unary = false;
    }
    if (is_unary) {
      uint32_t val = eval(p + 1, q, success);
      // Two's complement negation for uint32_t
      if (!*success) return 0;
      return (~val) + 1;
    }
  }

  // Handle pointer dereference: *expr
  if (tokens[p].type == TK_DEREF) {
    uint32_t addr = eval(p + 1, q, success);
    if (!*success) return 0;
    // Read 4 bytes from virtual address
    return vaddr_read(addr, 4);
  }


  if (p == q) {
      if (tokens[p].type == TK_NUM) {
      return (uint32_t)atoi(tokens[p].str);
    } else if (tokens[p].type == TK_HEX) {
      return (uint32_t)strtoul(tokens[p].str + 2, NULL, 16);
    } else if (tokens[p].type == TK_REG) {
      if (!strcmp(tokens[p].str, "$eax")) return cpu.eax;
      if (!strcmp(tokens[p].str, "$ebx")) return cpu.ebx;
      if (!strcmp(tokens[p].str, "$ecx")) return cpu.ecx;
      if (!strcmp(tokens[p].str, "$edx")) return cpu.edx;
      if (!strcmp(tokens[p].str, "$esp")) return cpu.esp;
      if (!strcmp(tokens[p].str, "$ebp")) return cpu.ebp;
      if (!strcmp(tokens[p].str, "$esi")) return cpu.esi;
      if (!strcmp(tokens[p].str, "$edi")) return cpu.edi;
      if (!strcmp(tokens[p].str, "$eip")) return cpu.eip;
      *success = false;
      return 0;
    } else {
      *success = false;
      return 0;
    }
   }


  bool matched;
  if (check_parentheses(p, q, &matched)) {
    return eval(p + 1, q - 1, success);
  }//here we strip the parentheses

  if (!matched) {
    // unmatched situation
    *success = false;
    return 0;
  }

  // divide and conquer
  int op = find_main_op(p, q);
  uint32_t val1 = eval(p, op - 1, success);
  uint32_t val2 = eval(op + 1, q, success);

  if (!*success) return 0;

  // calculation
  switch (tokens[op].type) {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': 
      if (val2 == 0) {
        *success = false;
        return 0;
      }
      return val1 / val2;

    case TK_EQ:  return (val1 == val2) ? 1 : 0;
    case TK_NE:  return (val1 != val2) ? 1 : 0;
    case TK_AND: return (val1 && val2) ? 1 : 0;
    
    default:
	*success=false;	    	
        assert(0);
  }
}

// the final external interface
uint32_t expr(char *e, bool *success) {
  *success = true;

  // analysis of morphology
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  // recursion
  return eval(0, nr_token - 1, success);
}



//uint32_t expr(char *e, bool *success) {
//  if (!make_token(e)) {
//    *success = false;
//    return 0;
//  }
//
//  /* TODO: Insert codes to evaluate the expression. */
//  TODO();
//
//  return 0;
//}
