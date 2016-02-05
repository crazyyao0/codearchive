// sudoku.cpp : Defines the entry point for the console application.
//
#include <stdlib.h>
#include "jpeg.h"


int main(int argc, char* argv[])
{
	JPEGDecode decode;
	decode.load("C:\\Users\\wyao\\Desktop\\tempc++\\sudoku\\1.jpg");
	decode.savebmp("images.bmp");
	system("pause");
	return 0;
}

