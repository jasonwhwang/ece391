#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystem.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

//divide by zero test
int idt_zero(){
	TEST_HEADER;

	int a,b,c;
	a = 132;
	b = 0;
	c = a/b;
	return c;
}

int trigger(){
	asm volatile("int $6");
	return 0;
}

int * dereference(){
		int *a = 0x0;
		int b;
		b = *a;
		return a;
}

/* Checkpoint 2 tests */
//print list of files in FS
void test_dir_read()
{
	//17 files
	void* buf_fill;
	dir_read(0, buf_fill, 0);
}

//test file_read() function
void test_file_read()
{
	int i;
	uint8_t file_buf[10000];
	clear();
	file_open((const uint8_t*) "hello");
	file_read(2, file_buf, 6000);

	for(i = 0; i < 6000; i++)
	{
		printf("%s", file_buf[i]);
	}
}

void rtc_test()
{
	int i;
	int j;
	for(i = 2; i <= 1024; i*=2)
	{
		rtc_write(i, 0, 0);
		for(j = 0; j < 2; j++)
		{
			rtc_read(0,0,0);
			printf("1");
		}
	}
	rtc_open(0);
	for(j = 0; j < 5; j++)
	{
		rtc_read(0,0,0);
		printf("3");
	}
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("zero_test", idt_zero());
	//TEST_OUTPUT("dereference", dereference());
	//test_dir_read();
	//test_file_read();
	rtc_test();
}
