#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  test_create_screen();

  while(1) { }
}


void test_screen_content()
{

}

void test_del_char()
{
  
}

void test_move_cursor()
{
  
}

void test_create_screen()
{
  int c = create_screen();
  set_focus(c);
}

void test_set_focus()
{
  
}