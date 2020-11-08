#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <omp.h>
#include <stdbool.h>

#include "list.h"

#define MAX_LINE 1024

int boxSize, rowSize, totalSize;
bool isSolved = false;

int ROW(int i) {return i / rowSize;}
int COL(int i) {return i % rowSize;}
int BOX(int r, int c) {return r / boxSize * boxSize + c / boxSize;}
int INT_TO_MASK(int num) {return (1 << (num - 1));}

bool can_solve(int* originSudoku);

void update_masks(int num, int row, int col, int* rowMask, int* colMask, int* boxMask);
void reset_masks(int num, int row, int col, int* rowMask, int* colMask, int* boxMask);

bool is_num_valid(int* rowMask, int* colMask, int* boxMask, int row, int col, int num);
bool is_all_threads_terminate(bool* isThreadTerminate, int threadNum);
bool dfs(List* searchList, int* sudokuGrid, int* rowMask, int* colMask, int* boxMask, int* originSudoku);
Node* rob_work(int tid, List** searchListArray, int** sudokuArray, int** rowMaskArray, int** colMaskArray, int** boxMaskArray, int* originSudoku);

int* read_grid(char* argv[]);
void print_result(int* sudoku);


int main(int argc, char* argv[])
{
	if(argc == 2)
	{
		omp_set_num_threads(8);
		int* originSudoku = read_grid(argv);
		print_result(originSudoku);

		if(can_solve(originSudoku))
			print_result(originSudoku);
		else
			printf("Sudoku not valid.\n");

		free(originSudoku);
	} 
	else
		printf("invalid argument.\n");

	return 0;
}


bool can_solve(int* originSudoku)
{
	int threadNum = omp_get_max_threads();
	int startPos = -1;  // the first cell we search

	List** searchListArray = (List**)malloc(threadNum * sizeof(List*));
	// use int as bit mask to help us to judge
	int** rowMaskArray = (int**)malloc(threadNum * sizeof(int*));
	int** colMaskArray = (int**)malloc(threadNum * sizeof(int*));
	int** boxMaskArray = (int**)malloc(threadNum * sizeof(int*));

	int** sudokuArray = (int**)malloc(threadNum * sizeof(int*));

	// help to check whether
	bool* isThreadTerminate = (bool*)malloc(threadNum * sizeof(bool));
	memset(isThreadTerminate, false, threadNum * sizeof(bool));

	// find begin position for DFS
	for(int i = 0; i < totalSize; ++i)
		if(originSudoku[i] == 0 && startPos == -1)
		{
			startPos = i;
			break;
		}
	printf("startPos: %d\n", startPos);

	// one thread one group of mask and one search list for saving the state of DFS
	#pragma omp parallel
	{
		int row, col, box, mask;
		int tid = omp_get_thread_num();
		searchListArray[tid] = init_list();

		rowMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));
		colMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));
		boxMaskArray[tid] = (int*)malloc(rowSize * sizeof(int));
		sudokuArray[tid] = (int*)malloc(totalSize * sizeof(int));

		memset(rowMaskArray[tid], 0, rowSize * sizeof(int));
		memset(colMaskArray[tid], 0, rowSize * sizeof(int));
		memset(boxMaskArray[tid], 0, rowSize * sizeof(int));

		for(int i = 0; i < totalSize; ++i)
			if(originSudoku[i] > 0)
			{
				row = ROW(i);
				col = COL(i);
				update_masks(originSudoku[i], row, col, rowMaskArray[tid], colMaskArray[tid], boxMaskArray[tid]);
				sudokuArray[tid][i] = originSudoku[i];
			}
		printf("thread %d\n", tid);
	}

	// parallel the DFS 
	#pragma omp parallel shared(startPos)
	{
		int tid = omp_get_thread_num();

		for(int startNum = 1; startNum <= rowSize; ++startNum)
			if(is_num_valid(rowMaskArray[tid], colMaskArray[tid], boxMaskArray[tid], 
				ROW(startPos), COL(startPos), startNum))
			printf("%d ", startNum);

		// Each thread begin parallel on startPos with different value
		// Then each thread begin to DFS, at least one thread will get valid result
		// If there are no more status to search, thread will look for work from other threads
		#pragma omp for nowait schedule(dynamic)
		for(int startNum = 1; startNum <= rowSize; ++startNum)
			if(is_num_valid(rowMaskArray[tid], colMaskArray[tid], boxMaskArray[tid], 
				ROW(startPos), COL(startPos), startNum))
			{
				Node* searchNode = new_node(startNum, startPos);
				printf("%d--------------------------------- \n", startNum);

				//push first state into DFS list, the list operation should be critical
				#pragma omp critical(list)
				insert_head(searchListArray[tid], searchNode);

				if(dfs(searchListArray[tid], sudokuArray[tid], rowMaskArray[tid], colMaskArray[tid], boxMaskArray[tid], originSudoku))
				{
					// only set the final result once
					#pragma omp critical(solved)
					if(!isSolved)
					{
						isSolved = true;
						for(int i = 0; i < totalSize; ++i)
							originSudoku[i] = sudokuArray[tid][i];
						printf("true\n");
					}

				}
				else
				{
					printf("false\n");
					isThreadTerminate[tid] = true;
				}
			}

		// idle thread will search for work from other thread list
		while(true)
		{
			if(isSolved)
				break;

			printf("robbbbbbbbbbbbbbbb\n");

			Node* robbedNode;
			#pragma omp critical(list)
			robbedNode = rob_work(tid, searchListArray, sudokuArray, rowMaskArray, colMaskArray, boxMaskArray, originSudoku);

			// DFS from robbed node
			if(robbedNode != NULL)
			{
				#pragma omp critical(list)
				insert_head(searchListArray[tid], robbedNode);

				if(dfs(searchListArray[tid], sudokuArray[tid], rowMaskArray[tid], colMaskArray[tid], boxMaskArray[tid], originSudoku))
				{
					// only set the final result once
					#pragma omp critical(solved)
					if(!isSolved)
					{
						isSolved = true;
						for(int i = 0; i < totalSize; ++i)
							originSudoku[i] = sudokuArray[tid][i];
						printf("true\n");
					}

				}

			}
			// check whether all the thread have finished their job
			// if so, there are no solution
			else if(is_all_threads_terminate(isThreadTerminate, threadNum))
			{
				for(int i = 0; i < threadNum; ++i)
					if(isThreadTerminate[i])
						printf("%d ", i);
				break;
			}
		}
	}

	// all threads finish search, release all the memory
	for(int i = 0; i < threadNum; ++i)
	{
		free(searchListArray[i]);
		free(sudokuArray[i]);
		free(rowMaskArray[i]);
		free(colMaskArray[i]);
		free(boxMaskArray[i]);
	}
	free(searchListArray);
	free(sudokuArray);
	free(rowMaskArray);
	free(colMaskArray);
	free(boxMaskArray);

	return isSolved;

}


bool dfs(List* searchList, int* sudokuGrid, int* rowMask, int* colMask, int* boxMask, int* originSudoku)
{
	// get the initial search state
	Node* searchNode;
	#pragma omp critical(list)
	searchNode = pop_head(searchList);

	int startPos = searchNode -> pos;

	// set the first state
	sudokuGrid[startPos] = searchNode -> num;
	update_masks(searchNode -> num, ROW(startPos), COL(startPos), rowMask, colMask, boxMask);

	int row, col;
	int curPos = startPos + 1;

	// keep DFS until final position
	while(curPos < totalSize)
	{
		
		// other thread have already found the solution
		if(isSolved)
		{
			printf("-----\n");
			return true;
		}

		if(originSudoku[curPos] == 0)
		{
			row = ROW(curPos);
			col = COL(curPos);

			int first_valid_num = -1;
			for(int num = 1; num <= rowSize; ++num)
				if(is_num_valid(rowMask, colMask, boxMask, row, col, num))
				{
					printf("valid: %d\n", num);
					// keep searching the first valid num
					if(first_valid_num == -1)
						first_valid_num = num;
					// push other valid nums into search list
					else
					{
						Node* newNode = new_node(num, curPos);
						#pragma omp critical(list)
						insert_head(searchList, newNode);

						printf("list size %d\n", get_list_size(searchList));
					}
				}
			printf("curPos: %d\n", curPos);
			printf("first num %d\n", first_valid_num);
			// write the valid num into sudoku, search next pos
			if(first_valid_num != -1)
			{
				update_masks(first_valid_num, row, col, rowMask, colMask, boxMask);
				sudokuGrid[curPos] = first_valid_num;
				print_result(sudokuGrid);
			}
			// if no valid number, the previous step must be wrong, need backtracking
			else
			{
				// no other work in current thread, this thread will find a job from other thread
				if(is_empty(searchList))
				{
					printf("empty!!!!!!!!!!!!!!!\n");
					// reset the mask and sudoku
					while(curPos >= startPos)
					{
						if(originSudoku[curPos] == 0)
						{
							reset_masks(sudokuGrid[curPos], row, col, rowMask, colMask, boxMask);
							sudokuGrid[curPos] = 0;
						}
						curPos--;
					}
					return 0;
				}
				// backtracking
				else 
				{
					// fetch a node from search list
					Node* prevNode;
					#pragma omp critical(list)
					prevNode = pop_head(searchList);

					// reset the status from current to the target
					for(int pos = curPos - 1; pos >= prevNode -> pos; --pos) // curPos is empty
						if(originSudoku[pos] == 0)
						{
							reset_masks(sudokuGrid[pos], ROW(pos), COL(pos), rowMask, colMask, boxMask);
							sudokuGrid[pos] = 0;
						}
					// set the target state
					curPos = prevNode -> pos;
					update_masks(prevNode -> num, ROW(curPos), COL(curPos), rowMask, colMask, boxMask);
					sudokuGrid[curPos] = prevNode -> num;

					printf("back pos: %d\n", curPos);
				}
			}
		}

		curPos++;
	}
	// return true when all cells are valid
	return true;  
}


Node* rob_work(int curThread, List** searchListArray, int** sudokuArray, int** rowMaskArray, int** colMaskArray, int** boxMaskArray, int* originSudoku)
{
	int threadNum = omp_get_max_threads();
	int robbedThread = -1, minPos = totalSize;
	Node* robbedNode = NULL;

	for(int tid = 0; tid < threadNum; ++tid)
	{	
		// find a condition which is closest to the search root
		// in order not to communicate too much
		if(!is_empty(searchListArray[tid]) && 
			get_tail_pos(searchListArray[tid]) < minPos)
		{
			minPos = get_tail_pos(searchListArray[tid]);
			robbedThread = tid;
		}
	}

	// if there is a valid node
	if(robbedThread != -1)
	{
		#pragma omp critical(rob)
		robbedNode = pop_tail(searchListArray[robbedThread]);

		// copy the mask and sudoku from robbed thread
		for(int i = 0; i < totalSize; ++i)
			sudokuArray[curThread][i] = sudokuArray[robbedThread][i];
		for(int i = 0; i < rowSize; ++i)
		{
			rowMaskArray[curThread][i] = rowMaskArray[robbedThread][i];
			colMaskArray[curThread][i] = colMaskArray[robbedThread][i];
			boxMaskArray[curThread][i] = boxMaskArray[robbedThread][i];
		}

		// reset the status ahead of robbed node status
		for(int i = totalSize - 1; i >= robbedNode -> pos; --i)
			if(originSudoku[i] == 0)
			{
				reset_masks(sudokuArray[curThread][i], ROW(i), COL(i), rowMaskArray[curThread], colMaskArray[curThread], boxMaskArray[curThread]);
				sudokuArray[curThread][i];
			}
	}

	return robbedNode;

}


void update_masks(int num, int row, int col, int* rowMask, int* colMask, int* boxMask)
{
	int box = BOX(row, col);
	int mask = INT_TO_MASK(num);

	rowMask[row] = rowMask[row] | mask;
	colMask[col] = colMask[col] | mask;
	boxMask[box] = boxMask[box] | mask;
}


void reset_masks(int num, int row, int col, int* rowMask, int* colMask, int* boxMask)
{
	int box = BOX(row, col);
	int mask = INT_TO_MASK(num);

	rowMask[row] = rowMask[row] ^ mask;
	colMask[col] = colMask[col] ^ mask;
	boxMask[box] = boxMask[box] ^ mask;
}


bool is_num_valid(int* rowMask, int* colMask, int* boxMask, int row, int col, int num)
{
	int mask = INT_TO_MASK(num);
	return !(rowMask[row] & mask) && 
		   !(colMask[col] & mask) && 
		   !(boxMask[BOX(row, col)] & mask);
}


bool is_all_threads_terminate(bool* isThreadTerminate, int threadNum)
{
	for(int i = 0; i < threadNum; ++i)
		if(!isThreadTerminate[i])
			return false;
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
	rowSize = boxSize * boxSize;
	totalSize = rowSize * rowSize;

	int* originSudoku = (int*)malloc(totalSize * sizeof(int));

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
				originSudoku[curIndex++] = atoi(tmp);
				tmpIndex = 0;
				memset(tmp, 0, sizeof(tmp));
			}
		}
	}

	fclose(f);
	return originSudoku;
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
























