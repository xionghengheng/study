原题链接：https://leetcode.com/problems/reverse-only-letters/

### 题意

给定一个字符串S，返回“反向”字符串，其中所有不是字母的字符都保留在同一个地方，并且所有字母都反转它们的位置。

**Example 1:**

```shell
Input: "ab-cd"
Output: "dc-ba"
```

**Example 2:**

```shell
Input: "a-bC-dEf-ghIj"
Output: "j-Ih-gfE-dCba"
```

**Example 3:**

```shell
Input: "Test1ng-Leet=code-Q!"
Output: "Qedo1ct-eeLg=ntse-T!"
```

**Note:**

1. `S.length <= 100`
2. `33 <= S[i].ASCIIcode <= 122` 
3. `S` doesn't contain `\` or `"`

### 思路

头尾指针分布从头尾遍历，只有同时为字母时，才进行互换，否则继续遍历

### 代码

```c++
class Solution {
public:
    string reverseOnlyLetters(string S) {
        int startIdx = 0;
        int endIdx = (int)(S.size() - 1);
        while (startIdx <= endIdx) {
            if (isChar(S[startIdx]) == false)
            {
                startIdx ++;
                continue;
            }
            if (isChar(S[endIdx]) == false)
            {
                endIdx --;
                continue;
            }
            char tmp = S[startIdx];
            S[startIdx] = S[endIdx];
            S[endIdx] = tmp;
            startIdx ++;
            endIdx --;
        }
        return S;
    }
    bool isChar(char c)
    {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'))
        {
            return true;
        }
        return false;
    }
};

//测试代码
int main()
{
    Solution so;
//    string S = "ab-cd";
//    string S = "a-bC-dEf-ghIj";
//    string S = "Test1ng-Leet=code-Q!";
//    string S = "";
//    string S = "-[]s;a";
    string S = "-a-";
    std::cout << so.reverseOnlyLetters(S) << std::endl;
}
```
