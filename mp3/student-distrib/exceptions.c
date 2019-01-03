#include "exceptions.h"
#include "lib.h"

//Exception calls
void e0() { exceptions(0); }
void e1() { exceptions(1); }
void e2() { exceptions(2); }
void e3() { exceptions(3); }
void e4() { exceptions(4); }
void e5() { exceptions(5); }
void e6() { exceptions(6); }
void e7() { exceptions(7); }
void e8() { exceptions(8); }
void e9() { exceptions(9); }
void e10() { exceptions(10); }
void e11() { exceptions(11); }
void e12() { exceptions(12); }
void e13() { exceptions(13); }
void e14() { exceptions(14); }
void e15() { exceptions(15); }
void e16() { exceptions(16); }
void e17() { exceptions(17); }
void e18() { exceptions(18); }
void e19() { exceptions(19); }
void e20() { exceptions(20); }
void e21() { exceptions(21); }
void e22() { exceptions(22); }
void e23() { exceptions(23); }
void e24() { exceptions(24); }
void e25() { exceptions(25); }
void e26() { exceptions(26); }
void e27() { exceptions(27); }
void e28() { exceptions(28); }
void e29() { exceptions(29); }
void e30() { exceptions(30); }
void e31() { exceptions(31); }

//Print outs for exceptions
void exceptions(int entry)
{
  switch(entry)
  {
    case 0:
      printf("EXCEPTION: Divide by Zero");
      break;
    case 1:
      printf("EXCEPTION: Debug");
      break;
    case 2:
      printf("EXCEPTION: NMI Interrupt");
      break;
    case 3:
      printf("EXCEPTION: Breakpoint");
      break;
    case 4:
      printf("EXCEPTION: OverFlow");
      break;
    case 5:
      printf("EXCEPTION: BOUND Range Exceed");
      break;
    case 6:
      printf("EXCEPTION: Invalid Opcode");
      break;
    case 7:
      printf("EXCEPTION: Device Not Available");
      break;
    case 8:
      printf("EXCEPTION: Double Fault");
      break;
    case 9:
      printf("EXCEPTION: Coprocessor Segment Overrun");
      break;
    case 10:
      printf("EXCEPTION: Invalid TSS");
      break;
    case 11:
      printf("EXCEPTION: Segment Not Present");
      break;
    case 12:
      printf("EXCEPTION: Stack Fault");
      break;
    case 13:
      printf("EXCEPTION: General Protection");
      break;
    case 14:
      printf("EXCEPTION: Page-Fault Exception");
      break;
    case 15:
      printf("EXCEPTION: Intel Reserved");
      break;
    case 16:
      printf("EXCEPTION: FPU Floating Point");
      break;
    case 17:
      printf("EXCEPTION: Alignment Check");
      break;
    case 18:
      printf("EXCEPTION: Machine-Check");
      break;
    case 19:
      printf("EXCEPTION: SIMD Floatin Point");
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
      printf("EXCEPTION: Intel Reserved");
  }
  //Spins after exception is handled
      asm volatile (".1: hlt; jmp .1;");

}
