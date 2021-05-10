/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <list.h>

#define NUM_COLUMNS 80
#define NUM_ROWS    25
#define NUM_SCREENS 1000

struct screen {
  int ID;			/* Screen ID. This MUST be the first field of the struct. */
  int PID;
  Byte x, y;
  char *content[NUM_COLUMNS][NUM_ROWS];
};

struct screen* current_screen;
struct screen* all_screens[1000];

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);

void delete();
void move(int x, int y);
void change_color();
struct screen* get_current_screen();
struct screen* new_screen(int pid);

#endif  /* __IO_H__ */
