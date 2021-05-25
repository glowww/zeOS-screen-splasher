#include <io.h>
#include <utils.h>
#include <list.h>

// Queue for blocked processes in I/O 
struct list_head blocked;

int to_number(char c){
  if (c >= 48 && c <= 57) return c - '0';
  return -1;
}

int to_color(char c){
  
  if (c == '0') return BLACK_COLOR;
  if (c == '1') return RED_COLOR;
  if (c == '2') return GREEN_COLOR;
  if (c == '3') return ORANGE_COLOR;
  if (c == '4') return BLUE_COLOR;
  if (c == '5') return MAGENTA_COLOR;
  if (c == '6') return CYAN_COLOR;
  if (c == '7') return WHITE_COLOR;

  return DEFAULT_COLOR;
}

int special_action(char *buffer, int i, int size, int *color, int screen_id){

  i++; // Skip "["

  if (buffer[i+2] == 'm'){
    if (buffer[i] == '3'){
        // Foreground
        *color = to_color(buffer[i+1]);
    }
    else if (buffer[i] == '4'){
        // Background
        *color %= 0x1000;
        *color |= to_color(buffer[i+1]) << 4;
    }

    return i+2;
  }

  int pos = 0;

  for (; i<size; i++){

    int n = to_number(buffer[i]);
  
    if (n >= 0){
      pos = pos*10 + n;
    }
    else {
      if (buffer[i] == ';'){
        move_x_to(screen_id, pos);
        pos = 0;
      }
      else if (buffer[i] == 'H'){
        move_y_to(screen_id, pos);
        return i;
      }
      else return i;
    }
  }

  return size;
}

int sys_write_console(char *buffer, int size, int screen_id)
{
  int i;
  int color = DEFAULT_COLOR;

  for (i=0; i<size; i++){
    
    if (buffer[i] == '\177'){
      delete(screen_id);
    }
    else if (buffer[i] == '\033'){
      i = special_action(buffer, i+1, size, &color, screen_id);
    }
    else{
      printc_with_color(buffer[i], color, screen_id);
    }
  }
  return size;
}
