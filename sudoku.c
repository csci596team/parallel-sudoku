#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <omp.h>

#include "list.h"

#define MAX_LINE 1024

int boxSize, rowSize, totalSize;

int ROW(int i) {return i / rowSize;}
int COL(int i) {return i % rowSize;}
int BOX(int r, int c) {return r / boxSize * boxSize + c / boxSize;}
int INT_TO_MASK(int num) {return (1 << (num - 1));}

bool can_solve(int* sudokuGrid);
int* read_grid(char* argv[]);
void print_result(int* sudoku);


int main(int argc, char* argv[])
{
	if(argc == 2)
	{
		omp_set_num_threads(4);
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
	int threadNum = omp_get_max_threads();
	printf("%d\n", threadNum);
	int beginPos = -1;  // the first cell we search

	List** searchListArray = (List**)malloc(threadNum * sizeof(List*));
	// use int as bit mask to help us to judge
	int** rowMaskArray = (int**)malloc(threadNum * sizeof(int*));
	int** colMaskArray = (int**)malloc(threadNum * sizeof(int*));
	int** boxMaskArray = (int**)malloc(threadNum * sizeof(int*));

	bool* originSudokuGrid = (bool*)malloc(totalSize * sizeof(bool));
	for(int i = 0; i < totalSize; ++i)
	{
		if(sudokuGrid[i] > 0)
			originSudokuGrid[i] = true;
		else
		{
			originSudokuGrid[i] = false;
			if(beginPos == -1)
				beginPos = i;
		}
	}

	// one thread one group of mask and one search list for saving the state of DFS
	#pragma omp parallel
	{
		int row, col, box, mask;
		int tid = omp_get_thread_num();
		searchListArray[tid] = init_list();

		rowMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));
		colMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));
		boxMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));

		memset(rowMaskArray[tid], 0, sizeof(rowMaskArray[tid]));
		memset(colMaskArray[tid], 0, sizeof(colMaskArray[tid]));
		memset(boxMaskArray[tid], 0, sizeof(boxMaskArray[tid]));

		for(int i = 0; i < totalSize; ++i)
			if(originSudokuGrid[i])
			{
				row = ROW(i);
				col = COL(i);
				box = BOX(row, col);
				mask = INT_TO_MASK(sudokuGrid[i]);
				rowMaskArray[tid][row] = rowMaskArray[tid][row] | mask;
				colMaskArray[tid][col] = colMaskArray[tid][col] | mask;
				boxMaskArray[tid][box] = boxMaskArray[tid][box] | mask;
			}
		printf("thread %d\n", tid);

	}

	// parallel the DFS stage
	// #pragma omp parallel shared(beginPos)
	// {
	// 	int tid = omp_get_thread_num();

	// }

	return true;

}


bool is_num_valid(int* rowMask, int* colMask, int* boxMask, int row, int col, int num)
{
	int mask = INT_TO_MASK(num);
	return !(rowMask[row] & mask) && 
		   !(colMask[col] & mask) && 
		   !(boxMask[BOX(row, col)] & mask);
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
























