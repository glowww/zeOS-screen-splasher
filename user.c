#include <libc.h>
#include <io.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  
  test_create_screen();
  //test_set_focus();
  // test_move_cursor();

  while(1) {
    
  }
}

void test_screen_content()
{

}

void test_del_char()
{
  
}

void test_move_cursor()
{
  user_print("\n\nTesting move cursor");

  //move(1, 0);
}

void test_create_screen()
{
  user_print("\n\nTesting create screen");

  int new_screens = 6;
  int number_of_screens = 2;
  int expected = number_of_screens + new_screens;

  for(int i = 0; i < new_screens; i++) number_of_screens = create_screen();
  
  assert(number_of_screens == expected);
}

void test_set_focus()
{
  user_print("\n\nTesting set focus");

  create_screen();
  create_screen();

  set_focus(4);
}

void user_print(char* s){
  write(1, s, strlen(s));
}

void assert(int bool){
  if (bool) user_print("\nTest succeeded");
  else user_print("\nTest failed");
}