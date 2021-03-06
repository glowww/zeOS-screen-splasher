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
#define SCREEN_SIZE NUM_ROWS*NUM_COLUMNS*sizeof(Word) 

#define BLACK_COLOR   0x0000
#define RED_COLOR     0x0400
#define GREEN_COLOR   0x0200
#define ORANGE_COLOR  0x0600
#define BLUE_COLOR    0x0100
#define MAGENTA_COLOR 0x0500
#define CYAN_COLOR    0x0300
#define WHITE_COLOR   0x0700
#define DEFAULT_COLOR GREEN_COLOR


typedef Word screen_content[NUM_COLUMNS*NUM_ROWS];

struct screen {
  int ID;			/* Screen ID. This MUST be the first field of the struct. */
  int PID;
  Byte x, y;
  void* content;
};

struct screen* current_screen;
struct screen all_screens[NUM_SCREENS];

int global_screen_id;

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c, int screen_id);
void printc_with_color(char c, int color, int screen_id);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void printk_screen(char *string, int screen_id);
void break_line(struct screen* screen);
void move_x_to(int screen_id, int x);
void move_y_to(int screen_id, int y);

void add_screen_info(struct screen *screen);
int create_new_screen(struct task_struct *c);
void delete(int screen_id);
void move(int x, int y, int screen_id);
void change_color();
int switch_task_by_pid(int current_pid, int new_pid);
struct screen * get_screen(int screen_id);


#endif  /* __IO_H__ */
