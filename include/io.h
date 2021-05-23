/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <list.h>
#include <sched.h>

#define NUM_COLUMNS 80
#define NUM_ROWS    25
#define NUM_SCREENS 10

#define BLACK_COLOR   0x0000
#define RED_COLOR     0x0400
#define GREEN_COLOR   0x0200
#define ORANGE_COLOR  0x0600
#define BLUE_COLOR    0x0100
#define MAGENTA_COLOR 0x0500
#define CYAN_COLOR    0x0300
#define WHITE_COLOR   0x0700
#define DEFAULT_COLOR GREEN_COLOR

struct screen {
  int ID;			/* Screen ID. This MUST be the first field of the struct. */
  int PID;
  Byte x, y;
  Word *content[NUM_COLUMNS*NUM_ROWS];
};

struct screen* current_screen;
struct screen all_screens[NUM_SCREENS];

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_with_color(char c, int color);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);

int create_new_screen(struct task_struct *c);
void delete();
void move(int x, int y);
void change_color();
struct screen* get_current_screen();


#endif  /* __IO_H__ */
