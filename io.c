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

void printc_with_color(char c, int color)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c));
  if (c=='\n')
  {
    current_screen->x = 0;
    current_screen->y=(current_screen->y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | color;
    Word *screen = (Word *) 0xb8000;
    
    screen[current_screen->y * NUM_COLUMNS + current_screen->x] = ch;

    if (++current_screen->x >= NUM_COLUMNS)
    {
      current_screen->x = 0;
      current_screen->y=(current_screen->y+1)%NUM_ROWS;
    }
  }
}

void printc(char c)
{
  printc_with_color(c, DEFAULT_COLOR);
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=current_screen->x;
  cy=current_screen->y;
  current_screen->x=mx;
  current_screen->y=my;
  printc(c);
  current_screen->x=cx;
  current_screen->y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}


void delete()
{
  move(-1, 0);
  printc(NULL);
  move(-1, 0);
}


void move(int x, int y)
{
  move_x(current_screen->x + x);
  move_y(current_screen->y + y);
}

void move_x(int x)
{
  current_screen->x = x;
  if (current_screen->x >= NUM_COLUMNS) current_screen->x = NUM_COLUMNS - 1;
  if (current_screen->x < 0) current_screen->x = 0;
}

void move_y(int y)
{
  current_screen->y = y;
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
  all_screens[global_screen_id].y = 0;

  // TODO: Store it in the process and reference it from all_screens
  c->screens[empty_screen] = &all_screens[global_screen_id];

  // Only if it's the first screen ever created
  if (global_screen_id == 1) current_screen = &all_screens[global_screen_id];

  return global_screen_id;
}

int focus_next_screen(struct task_struct *c)
{
  
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
      // Save screen content
      copy_data((Word *) 0xb8000, current_screen->content, sizeof(current_screen->content));

      // Change focus
      current_screen = &all_screens[i];

      // Load new screen content
      copy_data(current_screen->content, (Word *) 0xb8000, sizeof(current_screen->content));

      char a[100]; itoa(all_screens[i].ID, a, 100);
      char b[100]; itoa(all_screens[i].PID, b, 100);

      printk("\nScreen ID: "); printk(a); 
      printk("\nPID: "); printk(b);

      return c;
    }
  }

	return -ENOENT;
}