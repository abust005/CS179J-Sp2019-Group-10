/*
** Copyright (c) 2008-2013 http://www.Programmingembeddedsystems.com. This software
** is provided with no warranties.
*/


#ifndef _INC_STDARG
#define _INC_STDARG
#ifndef _VA_LIST_DEFINED
typedef char *	va_list;
#define _VA_LIST_DEFINED
#endif
#define _INTSIZEOF(n)	( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)	( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)	( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_copy(dest,src) (dest=src)
#define va_end(ap)	( ap = (va_list)0 )
#endif	/* _INC_STDARG */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _MALLOC_DEFINED
extern void * malloc(size_t _size);
#define _MALLOC_DEFINED
#endif



/*---------------------------------------------------------------------------*/
#ifndef RIMS_H
#define RIMS_H

/*---------------------------------------------------------------------------*/

#define O_MEM_ADDR 0xffffffff
#define I_MEM_ADDR 0xfffffffe
#define O_UART_ADDR 0xfffffffd
#define I_UART_ADDR 0xfffffffc
#define MAX_INPUT_SIZE 10000
#define putchar(c) putc(c)


/*---------------------------------------------------------------------------*/
volatile extern unsigned char A0;
volatile extern unsigned char A1;
volatile extern unsigned char A2;
volatile extern unsigned char A3;
volatile extern unsigned char A4;
volatile extern unsigned char A5;
volatile extern unsigned char A6;
volatile extern unsigned char A7;

volatile extern unsigned char B0;
volatile extern unsigned char B1;
volatile extern unsigned char B2;
volatile extern unsigned char B3;
volatile extern unsigned char B4;
volatile extern unsigned char B5;
volatile extern unsigned char B6;
volatile extern unsigned char B7;

volatile extern unsigned char A, B;

volatile extern unsigned char T, R;

volatile extern unsigned int __timer_ticks;
volatile extern unsigned int __cur_ticks;
volatile extern unsigned int __timer_enabled;

volatile extern unsigned int __uart_enabled;

volatile extern unsigned int __ISR1_isexecuting;//TimerISR
volatile extern unsigned int __ISR2_isexecuting;//RxISR

volatile extern unsigned char TxReady;
volatile extern unsigned char RxComplete;

volatile extern unsigned char __fail_flag;
volatile extern unsigned char __has_executed;

volatile extern unsigned int __interrupts_enabled;

static unsigned long nextrand = 1;

char DYNAMIC_MEMORY[1000];
unsigned int PLACE_DYNAMIC = 0;


/*---------------------------------------------------------------------------*/
//Implements functions from setjmp.h. Can be used for cooperative multitasking//
typedef struct {
	unsigned int __j_sp;
  	unsigned int __j_s0;
	unsigned int __j_s1;
	unsigned int __j_s2;
	unsigned int __j_s3;
	unsigned int __j_s4;
	unsigned int __j_s5;
	unsigned int __j_s6;
	unsigned int __j_s7;
	unsigned int __j_ra;
} jmp_buf[1];


extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

/*---------------------------------------------------------------------------*/

void TimerISR(void);
void RxISR(void);//called when we have a new character in the buffer

/*---------------------------------------------------------------------------*/

#define ASSERT( x ) if(! (x) ){ puts("Assertion failed at line "); puti(__LINE__); putc('!'); exit(); }

/*---------------------------------------------------------------------------*/

void __initialize__(void) {/* called to bootstrap */
	__fail_flag = 0;
	__has_executed = 1;
	__interrupts_enabled = 1;
}

/*---------------------------------------------------------------------------*/
int escapechar = 0;
void putc(char c) {
    *((char*)O_MEM_ADDR) = c;
}

/*---------------------------------------------------------------------------*/

void endl() {
	*((char*)O_MEM_ADDR) = '\r';
	*((char*)O_MEM_ADDR) = '\n';
}

/*---------------------------------------------------------------------------*/

void puts(/*const*/ char *s) {
    
    while( *s ) {
        *((char*)O_MEM_ADDR) = *s++;
    }
}

/*---------------------------------------------------------------------------*/

int abs(int x) {
    
    return x < 0 ? -x : x;
}

/*---------------------------------------------------------------------------*/

void puti(long int x){
	int i = 0, j;
	int neg = (x < 0);
	char out[12] = {0};
	char final[12] = {0};

	x = (-2 * neg + 1) * x;//strip any negative from x

	if(x == 0){
		puts("0");
	}

	else{
		while(x > 0){
			out[i++] = x % 10;
			x /= 10;
		}
		
		//now it's just reversed
		j = i - 1;
		i = 0;
		if(neg)
			final[i++] = '-';

		while(j >= 0){
			final[i++] = '0' + out[j--];
		}

		puts(final);
	}
}

/*---------------------------------------------------------------------------*/

void putiu(unsigned long int x){
	puti((signed long int) x);
}

/*---------------------------------------------------------------------------*/

void putsi(short int x){
	puti((long int) x);
}

/*---------------------------------------------------------------------------*/

void putsiu(unsigned short int x){
	puti((long int) x);
}

/*---------------------------------------------------------------------------*/

void *memset(void *d, /*const*/ unsigned char v, unsigned long count) {
	unsigned char *d_ptr = (unsigned char*)d;

	while( count-- ) {
		*d_ptr = v;
		d_ptr++;
	}

	return d;
}

/*---------------------------------------------------------------------------*/

void *memcpy(void *d, /*const*/ void *s, unsigned long t) {
    unsigned char *d_ptr = d;
	unsigned char *s_ptr = s;

    while( t-- ) {
        *(d_ptr) = *(s_ptr);
		d_ptr++; s_ptr++;
    }

    return d;
}

/*---------------------------------------------------------------------------*/

int strncmp(/*const*/ char *s1, /*const*/ char *s2, unsigned long n) {
    
    unsigned long i;
    
    for(i=0; i<n; i++) {
        
        if     ( s1[i] < s2[i] ) return -1;
        else if( s1[i] > s2[i] ) return +1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

unsigned long strlen(/*const*/ char *str) {
	unsigned long count = 0;

	while(str[count] != 0)
		count++;

	return count;
}

/*---------------------------------------------------------------------------*/

void srand(int x){
	nextrand = x;
}

/*---------------------------------------------------------------------------*/

int rand(void) {//FOR TESTING ONLY -- DEFINITELY NOT RANDOM
    nextrand = nextrand * 1103515245 + 12345;
    return (unsigned int)(nextrand/65536) % 32768;
}

/*---------------------------------------------------------------------------*/

void exit(){
	__fail_flag = 1;
}

/*---------------------------------------------------------------------------*/
void EnableInterrupts() {
	__interrupts_enabled = 1;
}

/*---------------------------------------------------------------------------*/
void DisableInterrupts() {
	__interrupts_enabled = 0;
}

/*---------------------------------------------------------------------------*/

void TimerSet(unsigned int miliseconds){

	if(__timer_enabled)
	{
		__timer_ticks = miliseconds;
	}

	else
	{
		__timer_enabled = 1;
		__timer_ticks = miliseconds;
	}

	__cur_ticks = 0;
}

/*---------------------------------------------------------------------------*/

extern int TimerRead(void);

/*---------------------------------------------------------------------------*/

void TimerOn(void){
	__timer_enabled = 1;
}

/*---------------------------------------------------------------------------*/

void TimerOff(void){
	__timer_enabled = 0;
}

/*---------------------------------------------------------------------------*/

void UARTOn(void){
	TxReady = 1;
	__uart_enabled = 1;
}

/*---------------------------------------------------------------------------*/

void UARTOff(void){
	__uart_enabled = 0;
	TxReady = 0;
}

/*---------------------------------------------------------------------------*/

void Sleep(void){
	__has_executed = 0;
	while(!__has_executed);
}

/*---------------------------------------------------------------------------*/

void __ISR1_Handler(void){
	__ISR1_isexecuting++;
	TimerISR();
	__has_executed = 1;
	//no need to reset __cur_ticks, simulator does that on its own
	__ISR1_isexecuting--;
}

void __RightAfterISR1Handler(void){}//just a dummy function 
//Keep this function immediately after __ISR1_Handler
//so we know the beginning address of the handler (LookUp(L"__ISR1_Handler"))
//AND the ending address of the handler			  (LookUp(L"RightAfterISR1Handler") - 1)
/*---------------------------------------------------------------------------*/

void __ISR2_Handler(void){
	__ISR2_isexecuting = 1;
	RxISR();
	__has_executed = 1;
	__ISR2_isexecuting = 0;
}//receive

void __RightAfterISR2Handler(void) {}

/*---------------------------------------------------------------------------*/

void DoNothing(){}

/*---------------------------------------------------------------------------*/

static void printchar(char **str, int c)
{
	extern void putchar(char c);
	
	if (str) {
		**str = c;
		++(*str);
	}
	else (void)putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}
static int print(char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if(*format == '\n' || *format == '\r'){
		endl();
		}
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			/*if( *format == 'f' ) {
				pc += printfl (out, va_arg( args, float ), 10, 1, width, pad, 'a');
				continue;
			}*/
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			if(*format == '\n')
			{
				++format;
				if(*format == 'r' || *format == 'n')
				{
					endl();
				}
				else
				{
					--format;
					printchar (out, *format);
				}
			}
			else
			{
			printchar (out, *format);
			}
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int printf(const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return print( 0, format, args );
}

int sprintf(char *out, const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return print( &out, format, args );
}

/*---------------------------------------------------------------------------*/
void getInput(char input[MAX_INPUT_SIZE]){
	//char input[MAX_INPUT_SIZE];
	int ph = 0; //placeholder
	char temp = *(char*)I_MEM_ADDR;
	
	if(temp != '\n' && temp != '\r'){
		input[ph] = temp;
		putc(input[ph]);
	} else{
		input[ph] = *(char*)I_MEM_ADDR;
		putc(input[ph]);
	}

	while(input[ph] != 13){
		ph++;
		input[ph] = *(char*)I_MEM_ADDR;
		putc(input[ph]);
	}
	input[ph] = 0;
	return;
}
/*---------------------------------------------------------------------------*/
char* getCharArray(int size){
	char* temp;
	if((PLACE_DYNAMIC + size + 1) > 1000){
		printf("Only Allowed 1000 Char of dynamic memory\n");
	}

	temp = &DYNAMIC_MEMORY[PLACE_DYNAMIC];
	PLACE_DYNAMIC += size;
	PLACE_DYNAMIC++;//For Null Char
	DYNAMIC_MEMORY[PLACE_DYNAMIC] = '\0';
	PLACE_DYNAMIC++;//For new String
	return temp;
}
/*---------------------------------------------------------------------------*/

int charToInt(char input[MAX_INPUT_SIZE], int b, int u){
	int ret = 0;
	int placeholder = 1;
	int neg = 0;
	int ph = 0;
	if(input[ph] == '-'){
		neg = 1;
		ph++;
	}
	//printf("u: %d, neg: %d, input: %s, ph: %d\n", u, neg, input, ph);
	if(b == 10){
		while(input[ph] != 0){
			if((input[ph] - '0') >= 0 && (input[ph] - '0') <= b-1){
				int i;
				int temp = ret;
				for(i = 0; i < b-1; i++){
					ret += temp;
				}
				ret += input[ph] - '0';
				ph++;
			} else{
				ph++;
			}
		}
		if(neg){
			if(!u){
				ret = ret * -1;
			}
		}
	}
	if(b == 16){
		while(input[ph] != 0){
			if (input[ph] >= '0' && input[ph] <= '9')
			   ret = (ret << 4) + (input[ph] - '0');
			else if (input[ph] >= 'A' && input[ph] <= 'F')
			    ret = (ret << 4) + (input[ph] - 'A' + 10);
			else if (input[ph] >= 'a' && input[ph] <= 'f')
			    ret = (ret << 4) + (input[ph] - 'a' + 10);
			ph++;
		}
	}
	//printf("ret: %d", ret);
	return ret;
}
int scan(char const *format, va_list args){
	char input[MAX_INPUT_SIZE];
	char* loopPtr;
	int i;
	getInput(input);
	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			if( *format == 's' ) {
				char **test = va_arg(args, char **);
				char *tempArray = getCharArray(strlen(input));				
				loopPtr = tempArray;
				for(i = 0; i < strlen(input); i++){
					*loopPtr = input[i];
					loopPtr++;
				}			
				*test = tempArray;	
				continue;
			}
			if( *format == 'd' ) {
				int* test = va_arg(args, int *);
				int tempInt = charToInt(input, 10, 0);
				*test = tempInt;
				continue;
			}
			if( *format == 'x' ) {
				int* test = va_arg(args, int *);
				int tempInt = charToInt(input, 16, 0);
				*test = tempInt;
				continue;
			}
			if( *format == 'X' ) {
				int* test = va_arg(args, int *);
				int tempInt = charToInt(input, 16, 0);
				*test = tempInt;
				continue;
			}
			if( *format == 'u' ) {
				int* test = va_arg(args, int *);
				int tempInt = charToInt(input, 10, 1);
				*test = tempInt;
				continue;
			}
			if(*format == 'c'){
				char* c = va_arg(args, char *);
				*c = input[0];
				continue;
			}
		}
	}
	va_end( args );
	return 0;
}
/*
int scanf(const char *format, ...)
{
        va_list args;
        
        va_start( args, format );
        return scan( format, args );
}
*/

/*
	The current implementation of asm for rims is dependent on the layout of this file
	it assumes that scan will be the last function in rims.h



*/







#endif //RIMS_H
