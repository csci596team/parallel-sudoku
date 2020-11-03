#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
// #include <omp.h>

#include "list.h"

#define MAX_LINE 1024

int boxSize, rowSize, totalSize;


bool can_solve(int* sudokuGrid);
int* read_grid(char* argv[]);
void print_result(int* sudoku);



int main(int argc, char* argv[])
{
	if(argc == 2)
	{
		int* sudokuGrid = read_grid(argv);

		if(can_solve(sudokuGrid))
			print_result(sudokuGrid);
		else
			printf("Sudoku not valid.\n");

		free(sudokuGrid);
	} 
	else
		printf("invalid argument.\n");

	return 0;
}


bool can_solve(int* sudokuGrid)
{
	// int threadNum = omp_get_max_threads();
	int threadNum = 1;

	List** searchListArray = (List**)malloc(threadNum * sizeof(List*));
	bool** rowMaskArray = (bool**)malloc(threadNum * sizeof(bool*));
	bool** colMaskArray = (bool**)malloc(threadNum * sizeof(bool*));
	bool** boxMaskArray = (bool**)malloc(threadNum * sizeof(bool*));

	bool* originSudokuArray = (bool*)malloc(totalSize * sizeof(bool));

	return true;

}


int* read_grid(char* argv[])
{
	FILE* f;
	if((f = fopen(argv[1], "r")) == NULL)
	{
		printf("unable to open input file %s.\n", argv[1]);
		exit(1);
	}

	fscanf(f, "%d\n", &boxSize);
	printf("%d", boxSize);
	rowSize = boxSize * boxSize;
	totalSize = rowSize * rowSize;

	int* sudokuGrid = (int*)malloc(totalSize * sizeof(int));

	// read the sudoku puzzle
	char* curLine = NULL;
	char tmp[3];
	int curIndex = 0, tmpIndex = 0;
	size_t charNum, mod = 1;

	while((charNum = getline(&curLine, &mod, f) ) != -1)
	{
		for(int i = 0; i < charNum; ++i)
		{
			if(isdigit(curLine[i]))
				tmp[tmpIndex++] = curLine[i];
			else if(tmpIndex > 0)  // inorder to avoid \0 and \n
			{
				sudokuGrid[curIndex++] = atoi(tmp);
				tmpIndex = 0;
				memset(tmp, 0, sizeof(tmp));
			}
		}
	}

	fclose(f);
	return sudokuGrid;
}

void print_result(int* sudoku)
{
	for(int i = 0; i < rowSize; ++i)
	{
		for(int j = 0; j < rowSize; ++j)
			printf("%d ", sudoku[i * rowSize + j]);
		printf("\n");
	}
}
























