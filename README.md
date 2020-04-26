# How to run
## Run test case 1 with model-checker
```Bash
> mkdir build
> cd build
> cmake -DMODEL_CHK_PATH=${model_checker_root} ..
# example:
> cmake -DMODEL_CHK_PATH=/home/mintyi/codework/model-checker ..
> make
> ./barrier -v -m 3
```
## Run test case 2 with pthread
```Bash
> cmake ..
> make
> ./barrier
```
# Why this implementation is correct
## Barrier tree design
Every thread has a corresponding `struct Node` in the tree. Each node has a local `threadSense`.
Because `threadSense` is a thread local variable, which means other threads don't have to see it,
I don't set it as an atomic type.

The whole tree has a `globalSense` bit that should be read by all threads, so `globalSense` is an atomic type.

### Tree building
Being different from the DFS building on the textbook, which is buggy when the number of threads is not 2^k-1,
I build the tree in a BFS way that can deal with arbitraty number of threads.
The only difference is that I treat the Barrier Tree as a complete tree rather than a full tree, so I need to
take care of the calculation of `children`.

Example:
1. #thread = 4, radix = 2
```
      n0
     / \
    n1  n2
   /
  n3
```
n0.children = 2, n1.children = 1, n2.children = 0, n3.children = 0

### Synchronization between threads
1. `globalSense` should be synchronized among all threads.
    It's read memory order is **required**. (mybarrier.hpp line:38)
    It's rmw memory order is **release**. (mybarrier.hpp line:40,41)
2. `count` should be synchronized among several threads (namely, a node and its children)
    It's read memory order is **required**. (mybarrier.hpp line:34)
    It's rmw memory order is **release**. (mybarrier.hpp line:46)
    When reset `count` = `children`, the memory order is **relaxed**. (mybarrier.hpp line:35)
3. Using release and required memory order in `await()` can also prevent rw before the barrier going after
the barrier and prevent rw after the barrier going before the barrier within a thread.

## Test cases
The first test case:
1. An shared array is initialized as all zeros.
2. The main thread create many thread, then reaches the barrier.
2. Every threads set their corresponding bit in an array to 1, then reach the barrier.
3. The main thread check whether all bits are 1.
If the barrier works wrong, the check will fail because data race.

The second test case:
1. An shared array is initialized as 0;
2. The main thread create many thread,
3. Every threads do nothing but wait on barrier.
4. The main thread set all position in the shared array as `target`, then reach the barrier
5. All threads check array[thread_id] == target, then wait on barrier
6. The main thread flip 'target', then reach the barrier
7. return to step 1 for 10000 iterations.

This test case guarantee that the barrier can be used repeatedly without error.
