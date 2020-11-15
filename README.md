# Parallel Sudoku Solver

In 2020 fall CSCI 596 final project, we build a parallel sudoku solver using OpenMP to parallelize backtracking algorithm on Sudoku.



### What is Sudoku
![standard-sudoku-example-from-wiki](pic/sudoku-example.png)
#### Rules (for standard Sudoku)

1. A standard Sudoku contains 81 cells in a 9 by 9 grid. 

2. Each cell contains a number from 1-9. 

3. Each number can only occur once in each row, column, and 3 by 3 box.

#### Commonly Algorithms

1. **Brute Force**: Require a long execution time.
2. **Humanistic Algorithm**: Quick solving method but it may not solve a Sudoku if not applying other methods.
3. **Backtracking**: Performing DFS with pruning strategy, guaranteeing a solution if it exists.



### Parallel DFS

Noticing that order of DFS won't change the solution of Sudoku, we could parallelize our DFS procedure.

Initially, each thread starts at a different state and perform single thread searching strategy.

Once a thread is idle (finishing its current work), it tries to find some works from other threads.

![parallel-dfs](pic/parallel-dfs.png)

#### An example of parallel DFS traversal

![dfs-step1](pic/dfs-step1.png)
![dfs-step2](pic/dfs-step2.png)
![dfs-step3](pic/dfs-step3.png)
![dfs-step4](pic/dfs-step4.png)
![dfs-step5](pic/dfs-step5.png)
![dfs-step6](pic/dfs-step6.png)
![dfs-step7](pic/dfs-step7.png)
![dfs-step8](pic/dfs-step8.png)


#### Race Condition

##### Race condition on work list

Each thread holds a work list to track its current state and record the remaining work. When one thread finishes its work, it tries to find other works from other threads by examining work lists of other threads.

Work list will be read and changed by different threads when a robbery occurs, which cause a race condition.

For each work list, we use mutex to ensure read-after-write consistency.

##### Race condition on mask updates

Once robbery occurred, one thread needs to copy masks from another thread. Data consistency must be guaranteed during the copy process. Thus, a mutex for each thread is needed.



### Experiments and Results

**Execution Time for 16x16 Sudoku**
![exec_time_16](pic/exec_time_16.png)

**Speed Up for 16x16 Sudoku**
![speed_up_16](pic/speed_up_16.png)

**Efficiency for 16x16 Sudoku**
![efficiency_16](pic/efficiency_16.png)



### Run

```shell
mkdir build
cd build
cmake ..
make
./sudoku-solver <sudoku-file>
```

