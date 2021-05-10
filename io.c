/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

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
    asm("movw %0, (%1)" : : "g"(ch), "g"(screen));
  }

  // TODO: Update content
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
  /*
  current_screen.content[current_screen.x][current_screen.y] = NULL;
  move(-1, 0);
  */
}


void move(int x, int y)
{
  /*
  current_screen.x += x;
  if (current_screen.x >= NUM_COLUMNS)
  {
    current_screen.x = NUM_COLUMNS - 1;
  }

  current_screen.y += y;
  if (current_screen.y >= NUM_ROWS)
  {
    current_screen.y = NUM_ROWS - 1;
  }
  */
}

int global_screen_id = 0;

struct screen* new_screen(int pid){

  char content[NUM_COLUMNS][NUM_ROWS] = {{}};

  struct screen* s = {
    ++global_screen_id,
    pid,
    0, 0,
    content
  };

  all_screens[global_screen_id] = s;

  return s;
}