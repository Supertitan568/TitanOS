#include "unit_tests.h"
#include "vga.h"

static void vga_test1(){
  // This tests the scrolling feature of the console
  printstr("VGA Test 1 (You should not see this)\n");
  for(int i=0; i < 26; i++){
    printstr("VGA Test 1(If don't see the first it passed)\n"); 
  }
  printstr("VGA Test 1 (This is the last)\n");
  printstr("VGA Test 1 (Just kidding)\n");
  printstr("VGA Test 1 (This is really the last)\n");
}

static void vga_test2(){
  // This tests the cursor of the screen
  // Get cursor position
  // Move cursor position to a random location
  // Compare to new cursor position
}

static void vga_test3(){
  for(int i = 0; i < 90; i++){
    printstr("a");
  }
}

void run_unit_tests(){
  vga_test1();
  vga_test3();
  return;
}

