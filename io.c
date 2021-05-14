/*
 * io.c - 
 */

#include <io.h>
#include <types.h>
#include <sched.h>
#include <errno.h>

/**************/
/** Screen  ***/
/**************/


/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c));
  if (c=='\n')
  {
    current_screen->x = 0;
    current_screen->y=(current_screen->y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
    DWord screen = 0xb8000 + (current_screen->y * NUM_COLUMNS + current_screen->x) * 2;
    
    if (++current_screen->x >= NUM_COLUMNS)
    {
      current_screen->x = 0;
      current_screen->y=(current_screen->y+1)%NUM_ROWS;
    }

    // current_screen->content[current_screen->x][current_screen->y] = c;

    asm("movw %0, (%1)" : : "g"(ch), "g"(screen));
  }

}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=current_screen->x;
  cy=current_screen->y;
  current_screen->x=mx;
  current_screen->y=my;
  printc(c);
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}


void delete()
{
  current_screen->content[current_screen->x][current_screen->y] = NULL;
  move(-1, 0);
  printc(NULL);
  move(-1, 0);
}


void move(int x, int y)
{
  current_screen->x += x;
  if (current_screen->x >= NUM_COLUMNS) current_screen->x = NUM_COLUMNS - 1;
  if (current_screen->x < 0) current_screen->x = 0;

  current_screen->y += y;
  if (current_screen->y >= NUM_ROWS) current_screen->y = NUM_ROWS - 1;
  if (current_screen->y < 0) current_screen->y = 0;
}

int global_screen_id = 0;

int create_new_screen(struct task_struct *c){

  int empty_screen = -1;
  for(int i = 0; i < 10 && empty_screen == -1; i++){
    if (c->screens[i] == NULL) empty_screen = i;
  }
  
  if (empty_screen == -1) return -ENOMEM;

  ++global_screen_id;

  all_screens[global_screen_id].ID = global_screen_id;
  all_screens[global_screen_id].PID = c->PID;
  all_screens[global_screen_id].x = 0;
  all_screens[global_screen_id].y = 19;

  for(int i = 0; i < NUM_COLUMNS; i++){
    for(int j = 0; j < NUM_ROWS; j++){
      all_screens[global_screen_id].content[i][j] = NULL;
    }
  }

  c->screens[empty_screen] = &all_screens[global_screen_id];

  // Only if it's the first screen ever created
  if (global_screen_id == 1) current_screen = &all_screens[global_screen_id];

  return global_screen_id;
}

int focus_next_screen(struct task_struct *c)
{

  // create_new_screen(c);
  
  int focus = 0;
  int num_of_screens = sizeof c->screens / sizeof *c->screens;

  for(int i = 0; i < num_of_screens && focus == 0; i++){
    if (c->screens[i] && c->screens[i]->ID == current_screen->ID) focus = i;
  }

  for(int i = focus+1; i < num_of_screens; i++){
    if (c->screens[i] && c->screens[i]->ID){
      return io_set_focus(c, c->screens[i]->ID);
    }
  }
  for(int i = 0; i < focus; i++){
    if (c->screens[i] && c->screens[i]->ID){
      return io_set_focus(c, c->screens[i]->ID);
    }
  }
}

int io_set_focus(struct task_struct *t, int c)
{
  for(int i = 0; i < NUM_SCREENS; i++)
  {
    if ( all_screens[i].PID == t->PID &&
        (c > global_screen_id && all_screens[i].ID > 0 || all_screens[i].ID == c)
      ) 
    {
      current_screen = &all_screens[i];

      for(int i = 0; i < NUM_COLUMNS; i++){
        for(int j = 0; j < NUM_ROWS; j++){
          printc_xy(i, j, current_screen->content[current_screen->x][current_screen->y]);
        }
      }

      char a[100]; itoa(all_screens[i].ID, a, 100);
      char b[100]; itoa(all_screens[i].PID, b, 100);
      printk("\nScreen ID: "); printk(a); 
      printk("\nPID: "); printk(b);

      return c;
    }
  }

	return -ENOENT;
}