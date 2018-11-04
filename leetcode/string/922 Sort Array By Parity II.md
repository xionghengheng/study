原题链接：https://leetcode.com/problems/sort-array-by-parity-ii/

### 题意

给定一个非负整数的数组A，A中的一半整数是奇数，而整数的一半是偶数。对数组进行排序，以便每当A [i]为奇数时，i为奇数; 只要A [i]是偶数，i就是偶数。您可以返回满足此条件的任何答案数组。

**Example 1:**

```
Input: [4,2,5,7]
Output: [4,5,2,7]
Explanation: [4,7,2,5], [2,5,4,7], [2,7,4,5] would also have been accepted.
```

**Note:**

1. `2 <= A.length <= 20000`
2. `A.length % 2 == 0`
3. `0 <= A[i] <= 1000`

### 思路

遍历A的过程中，按照数组下标为偶数则填偶数，数组下标为奇数则填奇数的原则，对A进行拷贝即可。

### 代码

```c++
class Solution {
public:
    vector<int> sortArrayByParityII(vector<int>& A) {
        vector<int> ret;
        ret.resize(A.size());
        int evenIdx = 0;
        int oddIdx = 1;
        for (auto& item : A) {
            if ((item & 1) == 0)
            {
                ret[evenIdx] = item;
                evenIdx += 2;
            }
            else
            {
                ret[oddIdx] = item;
                oddIdx += 2;
            }
        }
        return ret;
    }
};
int main()
{
    Solution so;
    vector<int> A = {4,2,5,7};
//    vector<int> A = {4,2,8,8,1,3,4,5,7,9};
//    vector<int> A = {1,4};
    vector<int> B = so.sortArrayByParityII(A);
    for (auto& b : B) {
        std::cout << b << std::endl;
    }
}
```



更为简洁的解法：

```c++

class Solution {
    public:
    vector<int> sortArrayByParityII(vector<int>& A) {
        int evenIdx = 0, oddIdx = 1;
        while (evenIdx < A.size()) {
            if ((A[evenIdx] & 1) == 1 && (A[oddIdx] & 1) == 0) {
                swap(A[i], A[j]);
            }
            if ((A[evenIdx] & 1) == 0) {
                evenIdx += 2;
            }
            if ((A[oddIdx] & 1) == 1) {
                oddIdx += 2;
            }
        }
        return A;
    }
};
```

