原题链接：https://leetcode.com/problems/find-first-and-last-position-of-element-in-sorted-array/

### 题意

给定一个升序的int数组，对于某个给定值，找到该值在数组中的起始位置和终止位置，算法的时间复杂度必须是O(logn)，如果没找到则返回[-1,1]

输入输出举例:

Input: nums = [5,7,7,8,8,10], target = 8

Output: [3,4]

Input: nums = [5,7,7,8,8,10], target = 6

Output: [-1,-1]

### 思路



### 代码

```c++
class Solution {
public:
        int findlow(vector<int> A,int n,int target){
        int mid=0;
        int start=0;
        int end=n-1;
        while(start<=end){
            mid=(start+end)/2;
            if(A[mid]<target) start=mid+1;
            else if(A[mid]>target) end=mid-1;
            else{
                if(mid==0||(mid>0&&A[mid]==target&&A[mid-1]!=target))
                    return mid;
                else end=mid-1;
            }
        }
        return -1;
    }
    int findhigh(vector<int> A,int n,int target){
        int mid;
        int start=0;
        int end=n-1;
        while(start<=end){
            mid=(start+end)/2;
            if(A[mid]<target) start=mid+1;
            else if(A[mid]>target) end=mid-1;
            else{
                if(mid==n-1||(mid<n-1&&A[mid]==target&&A[mid+1]!=target))
                    return mid;
                else start=mid+1;
            }
        }
        return -1;
    }
    vector<int> searchRange(vector<int>& nums, int target) {
        vector<int> result;
        result.push_back(-1);
        result.push_back(-1);
        if(nums.size()==0) return result;
        result.clear();
        int l=findlow(nums,nums.size(),target);
        int h=findhigh(nums,nums.size(),target);
        result.push_back(l);
        result.push_back(h);
        return result;
    }
};
```
