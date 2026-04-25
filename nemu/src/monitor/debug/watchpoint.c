#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

//the "static" of the next 2 line has been deleted,which here are in thr begin.
WP wp_pool[NR_WP];
WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(!free_)
	  assert(0);
  WP* cur=free_;
  free_=free_->next;
  cur->next=head;
  head=cur;
  return cur;
};

void free_wp(WP *wp){
  if(!wp) 
	  return;
  WP *front=head;
  if(front == wp){
    head=head->next;
  }
  else{ 
	  while(front->next != wp){
		  front=front->next;
	  }; 
  front->next = wp->next;
  };
  wp->used=false;
  wp->next = free_;
  free_=wp;
};

// the next block is to finish the "info w" command.
void wp_display(void) {
    printf("Num     Type           Disp Enb Address            What\n");

    WP *p = head;
    while (p != NULL) {
        if (p->used) {
            printf("%-8dwatchpoint    keep y   0x%08x         %s\n",
                   p->NO,
                   p->old_val,
                   p->expr);
        }
        p = p->next;
    }
}
