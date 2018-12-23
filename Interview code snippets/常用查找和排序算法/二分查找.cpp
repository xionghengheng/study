#include <iostream>
#include <vector>
using namespace std;

//二分查找-循环版本
int binarySerch(const vector<int>& arr, int target)
{
    int begIdx = 0;
    int endIdx = (int)(arr.size() - 1);
    while (begIdx <= endIdx)
    {
        int midIdx = (begIdx + endIdx) / 2;
        if (target > arr[midIdx]) begIdx = midIdx + 1;
        else if (target < arr[midIdx]) endIdx = midIdx - 1;
        else return midIdx;
    }
    return -1;
}

//二分查找-递归版本
int binarySerchRecursion(const vector<int>& arr, int target, int begIdx, int endIdx)
{
    int midIdx = (begIdx + endIdx) / 2;
    if (begIdx > endIdx)
    {
        return -1;
    }
    if (target > arr[midIdx])
    {
        return binarySerchRecursion(arr, target, midIdx + 1, endIdx);
    }
    else if (target < arr[midIdx])
    {
        return binarySerchRecursion(arr, target, midIdx, endIdx - 1);
    }
    else
    {
        return midIdx;
    }
    return -1;
}

int binarySerch2(const vector<int>& arr, int target)
{
    return binarySerchRecursion(arr, target, 0, (int)(arr.size() - 1));
}

int main()
{
    vector<int> arr = {1,13,20,57,92,109,308,1023,4502};
    std::cout << "二分查找结果索引=" << binarySerch(arr, 308) << std::endl;
    std::cout << "二分查找结果索引=" << binarySerch(arr, 0) << std::endl;
    std::cout << "二分递归查找结果索引=" << binarySerch2(arr, 1023) << std::endl;
    std::cout << "二分递归查找结果索引=" << binarySerch2(arr, 4501) << std::endl;
    return 1;
}
