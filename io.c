/*
 * io.c - 
 */

#include <io.h>
#include <types.h>
#include <sched.h>
#include <errno.h>
#include <libc.h>

/**************/
/** Screen  ***/
/**************/

global_screen_id = 1;
Word *physical_screen = 0xb8000;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc_with_color(char c, int color, int screen_id)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c));
  
  struct screen *screen;
  int use_current_screen = screen_id <= 1 || screen_id == current_screen->ID;

  if (use_current_screen) screen = current_screen;
  else screen = &all_screens[screen_id];

  if (c == '\n') {
    break_line(screen);
    return;
  }

  Word ch = (Word) (c & 0x00FF) | color;

  if (use_current_screen){
    physical_screen[screen->y * NUM_COLUMNS + screen->x] = ch;
  }
  else {
    ((Word *) screen->content)[screen->y * NUM_COLUMNS + screen->x] = ch;
  }
  
  if (++screen->x >= NUM_COLUMNS) break_line(screen);
}

void break_line(struct screen* screen){
  screen->x = 0;
  screen->y=(screen->y+1)%NUM_ROWS;
}

void printc(char c, int screen_id)
{
  printc_with_color(c, DEFAULT_COLOR, screen_id);
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=current_screen->x;
  cy=current_screen->y;
  current_screen->x=mx;
  current_screen->y=my;
  printc(c, 0);
  current_screen->x=cx;
  current_screen->y=cy;
}

void printk(char *string)
{
  printk_screen(string, 0);
}

void printk_screen(char *string, int screen_id)
{
  for (int i = 0; string[i]; i++) printc(string[i], screen_id);
}

void delete()
{
  move(-1, 0);
  printc(NULL, 0);
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
  if (global_screen_id == 2) current_screen = &all_screens[global_screen_id];

  add_screen_info(&all_screens[global_screen_id]);

  return global_screen_id;
}

void add_screen_info(struct screen *screen){

  char sid[10], pid[10]; 

  itoa(screen->ID, sid);
  itoa(screen->PID, pid);

  screen->x = 71 - strlen(sid);
  screen->y = 22;
  
  printk_screen("Screen: ", screen->ID); 
  printk_screen(sid, screen->ID); 

  screen->x = 70 - strlen(pid);
  screen->y++;

  printk_screen("Process: ", screen->ID); 
  printk_screen(pid, screen->ID);

  screen->x = 0;
  screen->y = 0;
}

// Por algún motivo no es capaz de encontrar strlen() en libc.h pero sí
// es capaz de encontrar atoi() ¯\_(ツ)_/¯
int strlen(char *a)
{
  int i=0;
  while (a[i]!=0) i++;
  return i;
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
      return focus_screen(c, c->screens[i]->ID);
    }
  }
  for(int i = 0; i < focus; i++){
    if (c->screens[i] && c->screens[i]->ID){
      return focus_screen(c, c->screens[i]->ID);
    }
  }
}

int focus_screen(struct task_struct *t, int c)
{
  for(int i = 0; i < NUM_SCREENS; i++)
  {
    if ( all_screens[i].PID == t->PID &&
        (c > global_screen_id && all_screens[i].ID > 0 || all_screens[i].ID == c)
      ) 
    {
      // Save screen content
      copy_data(physical_screen, current_screen->content, sizeof(current_screen->content));

      // Change focus
      current_screen = &all_screens[i];

      // Load new screen content
      copy_data(current_screen->content, physical_screen, sizeof(current_screen->content));

      return c;
    }
  }

	return -ENOENT;
}