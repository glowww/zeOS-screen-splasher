#include <libc.h>
#include <io.h>

// Screen 1 is the physical one and Screen 2 is the one from the init process
#define FIRST_SCREEN 2

void test_create_screen();
void test_set_focus();
void test_move_cursor();
void test_del_char();
void test_colors();
void test_write_in_background();
void test_create_screen_in_threads();
void test_set_focus_from_child();
void test_close_screen();
void test_close_screen_in_threads();
void assert(int bool);
void user_print(char* s);


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  // Change this variable to select a different test
  int test_selected = 1;

  if (test_selected == 1) test_create_screen();
  else if (test_selected == 2) test_set_focus();
  else if (test_selected == 3) test_move_cursor();
  else if (test_selected == 4) test_del_char();
  else if (test_selected == 5) test_colors();
  else if (test_selected == 6) test_write_in_background();
  else if (test_selected == 7) test_create_screen_in_threads();
  else if (test_selected == 8) test_set_focus_from_child();
  else if (test_selected == 9) test_close_screen();
  else if (test_selected == 10) test_close_screen_in_threads();

  while(1) {}
}

void test_del_char()
{
  create_screen();
  set_focus(3);
  user_print("\n\n");

  char* hidden = "You will never see this.";
  user_print(hidden);

  char to_delete[24];
  for (int i = 0; i < 24; i++) to_delete[i] = '\177';

  user_print(to_delete);
  user_print("See what?");
}

void test_colors()
{
  create_screen();
  set_focus(3);
  user_print("\n\n\033[31mT\033[32me\033[33ms\033[34mt\033[35mi\033[36mn\033[37mg \033[31mc\033[32mo\033[33ml\033[34mo\033[35mr\033[36ms\n\n");

  user_print("\033[37m BLACK   RED   GREEN ORANGE  BLUE  MAGENTA CYAN  WHITE  DEFAULT  \n");
  user_print("\033[30mBLACK  \033[41mBLACK  \033[42mBLACK  \033[43mBLACK  \033[44mBLACK  \033[45mBLACK  \033[46mBLACK  \033[47mBLACK  \033[49mBLACK  \n");
  user_print("\033[31mRED    \033[41mRED    \033[42mRED    \033[43mRED    \033[44mRED    \033[45mRED    \033[46mRED    \033[47mRED    \033[49mRED    \n");
  user_print("\033[32mGREEN  \033[41mGREEN  \033[42mGREEN  \033[43mGREEN  \033[44mGREEN  \033[45mGREEN  \033[46mGREEN  \033[47mGREEN  \033[49mGREEN  \n");
  user_print("\033[33mORANGE \033[41mORANGE \033[42mORANGE \033[43mORANGE \033[44mORANGE \033[45mORANGE \033[46mORANGE \033[47mORANGE \033[49mORANGE \n");
  user_print("\033[34mBLUE   \033[41mBLUE   \033[42mBLUE   \033[43mBLUE   \033[44mBLUE   \033[45mBLUE   \033[46mBLUE   \033[47mBLUE   \033[49mBLUE   \n");
  user_print("\033[35mMAGENTA\033[41mMAGENTA\033[42mMAGENTA\033[43mMAGENTA\033[44mMAGENTA\033[45mMAGENTA\033[46mMAGENTA\033[47mMAGENTA\033[49mMAGENTA\n");
  user_print("\033[36mCYAN   \033[41mCYAN   \033[42mCYAN   \033[43mCYAN   \033[44mCYAN   \033[45mCYAN   \033[46mCYAN   \033[47mCYAN   \033[49mCYAN   \n");
  user_print("\033[37mWHITE  \033[41mWHITE  \033[42mWHITE  \033[43mWHITE  \033[44mWHITE  \033[45mWHITE  \033[46mWHITE  \033[47mWHITE  \033[49mWHITE  \n");
  user_print("\033[39mDEFAULT\033[41mDEFAULT\033[42mDEFAULT\033[43mDEFAULT\033[44mDEFAULT\033[45mDEFAULT\033[46mDEFAULT\033[47mDEFAULT\033[49mDEFAULT\n");
}

void test_move_cursor()
{
  create_screen();
  set_focus(3);
  user_print("\n\nTesting move cursor");

  user_print("\033[2;6HA\033[3;7HB\033[4;8HC");
  user_print("\033[5;9HD\033[6;10HE\033[7;11HF");
  user_print("\033[6;12HG\033[5;13HH\033[4;14HI");
  user_print("\033[3;15HJ\033[2;16HK");
}

void test_create_screen()
{
  user_print("\n\nTesting create screen");

  int new_screens = 6;
  int number_of_screens = 2;
  int expected = number_of_screens + new_screens;

  for(int i = 0; i < new_screens; i++){
    number_of_screens = create_screen();
  }
  
  assert(number_of_screens == expected);
}

void test_set_focus()
{
  user_print("\n\nTesting set focus");

  create_screen();
  create_screen();

  set_focus(4);
  user_print("\nYou will only read this in screen #4");
}

void test_write_in_background(){

  user_print("\n\nTesting write in background");

  create_screen(); 
  create_screen();
  
  char *string = "\n\nThis was written in the background";

  write(4, string, strlen(string));
  set_focus(4);
}

void test_create_screen_in_threads(){
  
  user_print("\n\nTesting create screens in threads");
  
  create_screen();

  if (fork() == 0){
    create_screen();
    if (fork() == 0){
      create_screen();
      if (fork() == 0){
        create_screen();
        if (fork() == 0){
          create_screen();
          if (fork() == 0){
            create_screen();
            if (fork() == 0){
              assert(create_screen() == 9);
            }
          }
        }
      }
    }
  }
}

void test_set_focus_from_child(){
  
  user_print("\n\nTesting set focus from child");

  create_screen();
  create_screen();

  if (fork() == 0){
    create_screen();
    set_focus(5);
    user_print("\nChild process changed focus and wrote this");
  }

}

void test_close_screen(){
  
  user_print("\n\nTesting close screen");

  create_screen();
  create_screen();
  create_screen();

  close(4);

  set_focus(3);

  char *string = "\n\nScreen #4 doesn't exist anymore";
  write(3, string, strlen(string));

  string = "\n\nScreen #3 is right";
  write(5, string, strlen(string));
}

void test_close_screen_in_threads(){
  
  user_print("\n\nTesting close screen in threads");

  create_screen();
  create_screen();

  if (fork() == 0){
    close(4);
    set_focus(create_screen());
    user_print("\nChild process killed screen #4 :(");
  }
}

void user_print(char* s){
  write(1, s, strlen(s));
}

void assert(int bool){
  if (bool) user_print("\nTest succeeded");
  else user_print("\nTest failed");
}