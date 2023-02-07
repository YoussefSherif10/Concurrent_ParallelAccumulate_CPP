#include <iostream>
#include <thread>
#include <algorithm>
#include <numeric>
#include <vector>

using namespace std;

template<typename Iterator, typename T>
void accumulateBlock(Iterator first, Iterator last, T &result){
    result = accumulate(first, last, result);
}

template<typename Iterator, typename T>
/**
 * calculate the sum of range in concurrent.
 * @param first : pos of the first element in the range
 * @param last : pos of the last element in the range
 * @param init : initial value of summation
 * @return the sum of all elements in the range with the initial value
 */
T parallelAccumulate(Iterator first, Iterator last, T init){
    // get the length of the range.
    const unsigned long length = distance(first, last);

    if (!length)
        return init;

    // determine the number of threads in the runtime
    const unsigned long hardware_threads = thread::hardware_concurrency();
    const unsigned long numThreads =
            min(hardware_threads != 0 ? hardware_threads : 2, length);

    // determine the block size to be advanced in each step
    const unsigned long blockSize = length / numThreads;

    // vectors to store results from the threads and to manage the threads
    // used vectors as they are initialized to zeros by default
    vector<T> results(numThreads);
    vector<thread> threads(numThreads-1);

    Iterator blockStart = first;
    for (unsigned short i = 0 ; i < (numThreads-1); ++i){
        // get the starting point and advance block size steps
        Iterator blockEnd = blockStart;
        advance(blockEnd, blockSize);

        // start thread with range of a block and get result stored in a vector
        threads[i] = thread(accumulateBlock<Iterator, T>, blockStart, blockEnd, ref(results[i]));

        // set the next starting point
        blockStart = blockEnd;
    }

    // the last main thread
    accumulateBlock<Iterator, T>(blockStart, last, results[numThreads - 1]);

    // wait for all threads to finish
    for (auto &i: threads) {
        i.join();
    }

    // return the sum of the result vector and the initial value
    return accumulate(results.begin(), results.end(), init);

}

int main() {
    vector<int> numbers;

    for (int i = 0; i < 100; ++i) {
        numbers.push_back(i);
    }

    int sum = parallelAccumulate(numbers.begin(), numbers.end(), 0);
    cout << sum << endl;

    return 0;
}
