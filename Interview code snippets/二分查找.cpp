//二分查找-循环版本
int binarySerch(vector<int> arr, int target)
{
    int begIdx = 0;
    int endIdx = (int)(arr.size() - 1);
    while (begIdx <= endIdx)
    {
        int mid = (begIdx + endIdx) / 2;
        if (target > arr[mid]) begIdx = mid + 1;
        else if (target < arr[mid]) endIdx = mid - 1;
        else return mid;
    }
    return -1;
}

//二分查找-递归版本
int binarySerch2(vector<int> arr, int target)
{
    return -1;
}

int main()
{
    vector<int> arr;
    arr.push_back(1);
    arr.push_back(20);
    arr.push_back(57);
    arr.push_back(92);
    arr.push_back(109);
    arr.push_back(308);
    arr.push_back(1023);
    cout << "二分查找结果索引=" << binarySerch(arr, 1) << endl;
    cout << "二分查找结果索引=" << binarySerch2(arr, 1) << endl;
    return 1;
}
