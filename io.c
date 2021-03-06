/*
 * io.c - 
 */

#include <io.h>
#include <types.h>
#include <sched.h>
#include <errno.h>
#include <libc.h>
#include <mm.h>

/**************/
/** Screen  ***/
/**************/

global_screen_id = 1;
Word *physical_screen = (Word *) 0xb8000;

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

void delete(int screen_id)
{
  move(-1, 0, screen_id);
  printc(NULL, screen_id);
  move(-1, 0, screen_id);
}

void move(int x, int y, int screen_id)
{
  struct screen *screen = get_screen(screen_id);
  move_x_to(screen_id, screen->x+x);
  move_y_to(screen_id, screen->y+y);
}

void move_x_to(int screen_id, int x)
{
  struct screen *screen = get_screen(screen_id);
  screen->x = x;
  if (screen->x >= NUM_COLUMNS) screen->x = NUM_COLUMNS - 1;
  if (screen->x < 0) screen->x = 0;
}

void move_y_to(int screen_id, int y)
{
  struct screen *screen = get_screen(screen_id);
  screen->y = y;
  if (screen->y >= NUM_ROWS) screen->y = NUM_ROWS - 1;
  if (screen->y < 0) screen->y = 0;
}

int create_new_screen(struct task_struct *c){

  int empty_screen = -1;
  for(int i = 0; i < SCREENS_PER_TASK && empty_screen == -1; i++){
    if (c->screens[i] == NULL) empty_screen = i;
  }
  if (empty_screen == -1) return -ENOMEM;

  int pag = alloc_frame();
  if (pag == -1) return -ENOMEM;

  int pag_logica = PAG_LOG_INIT_DATA + NUM_PAG_DATA + 20 + empty_screen;

  page_table_entry* PT = get_PT(c);
  set_ss_pag(PT, pag_logica, pag);

  PT[pag_logica].bits.user = 0;

  ++global_screen_id;

  all_screens[global_screen_id].ID = global_screen_id;
  all_screens[global_screen_id].PID = c->PID;
  all_screens[global_screen_id].x = 0;
  all_screens[global_screen_id].y = 0;
  all_screens[global_screen_id].content = (pag_logica<<12);

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

// Por alg??n motivo no es capaz de encontrar strlen() en libc.h pero s??
// es capaz de encontrar atoi() ??\_(???)_/??
int strlen(char *a)
{
  int i=0;
  while (a[i]!=0) i++;
  return i;
}

int focus_next_screen(struct task_struct *c)
{
  int focus = current_screen->ID + 1;

  while (focus <= global_screen_id && all_screens[focus].ID == -1){
    focus++;
  }

  if (focus > global_screen_id) focus = 2;

  while (focus <= global_screen_id && all_screens[focus].ID == -1){
    focus++;
  }
  
  return focus_screen(c, focus);
}

int focus_screen(struct task_struct *c, int screen_id)
{
  if (screen_id > global_screen_id || screen_id < 2) return -ENOENT;

  int current_pid = switch_task_by_pid(c->PID, current_screen->PID);

  // Save screen content
  copy_data(physical_screen, current_screen->content, SCREEN_SIZE);

  // Change focus
  current_screen = &all_screens[screen_id];

  switch_task_by_pid(current_pid, current_screen->PID);

  // Load new screen content
  copy_data(current_screen->content, physical_screen, SCREEN_SIZE);

  return screen_id;
}

int close_screen(struct task_struct *c, int screen_id){
  
  for(int i = 0; i < SCREENS_PER_TASK; i++){
    if (c->screens[i]->ID == screen_id){
      
      c->screens[i] = NULL;
      
      page_table_entry* PT = get_PT(c);
      free_frame(get_frame(PT, PAG_LOG_INIT_SCREENS+i));
      del_ss_pag(PT, PAG_LOG_INIT_SCREENS+i);

      all_screens[screen_id].ID  = -1;
      all_screens[screen_id].PID = -1;

      return 1;
    }
  }

  return -ENOENT;
}

int switch_task_by_pid(int current_pid, int new_pid)
{
  if (current_pid == new_pid) return new_pid;

  set_cr3(get_DIR(get_task_by_pid(new_pid)));

  return new_pid;
}

struct screen * get_screen(int screen_id){

  if (screen_id <= 1) return current_screen;
  return &all_screens[screen_id];
}