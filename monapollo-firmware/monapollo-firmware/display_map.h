#ifndef DISPLAY_MAP_H
#define DISPLAY_MAP_H

//define digit anodes
#define ONES	0b00001000
#define TENS	0b00000001
#define HUNDS	0b00000010
#define THOUS	0b00000100

//define digit cathodes (current sink, active low)
#define a		0b00000100
#define b		0b00000001
#define c		0b00010000
#define d		0b01000000
#define e		0b10000000
#define f		0b00000010
#define g		0b00001000
#define dp		0b00100000

//define decimal digits
#define ZERO	(a | b | c | d | e | f)
#define ONE		(b | c)
#define TWO		(a | b | d | e | g)
#define THREE	(a | b | c | d | g)
#define FOUR	(b | c | f | g)
#define FIVE	(a | c | d | f | g)
#define SIX		(a | c | d | e | f | g)
#define SEVEN	(a | b | c)
#define EIGHT	(a | b | c | d | e | f | g)
#define NINE	(a | b | c | d | f | g)


//define hex digits
#define A		(a | b | c | e | f | g)
#define B		(c | d | e | f | g)
#define C		(a | d | e | f)
#define D		(b | c | d | e | g)
#define E		(a | d | e | f | g)
#define F		(a | e | f | g)

#endif