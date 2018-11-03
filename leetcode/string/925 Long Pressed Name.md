原题链接：https://leetcode.com/problems/long-pressed-name/

### 题意

你的朋友正在键盘上输入他的名字。 有时，在键入字符c时，按键可能会被长按，键入的字符将被输入1次或更多次。您检查键盘的键入字符时， 如果该字符可能是您的朋友的姓名，则返回True，其中一些字符（可能没有）被长按。name为您朋友的真实名字，typed为输入的字符，可能长按，也可能没有。

**Example 1:**

```shell
Input: name = "alex", typed = "aaleex"
Output: true
Explanation: 'a' and 'e' in 'alex' were long pressed.
```

**Example 2:**

```shell
Input: name = "saeed", typed = "ssaaedd"
Output: false
Explanation: 'e' must have been pressed twice, but it wasn't in the typed output.
```

**Example 3:**

```shell
Input: name = "leelee", typed = "lleeelee"
Output: true
```

**Example 4:**

```shell
Input: name = "laiden", typed = "laiden"
Output: true
Explanation: It's not necessary to long press any character.
```

### 思路

两个字符串的【首轮比较】一定相等，如果不相等则肯定不为长按字符串，如果相等，则将typed字符串继续往后迭代，直到不相等为止。

注意：

- 如果name字符串有多个相同字符，则在typed字符串在【次轮迭代】时，需要break本次循环，将name字符串往后移一个字符串

- 当两字符串校对过程中，typed先溢出，则返回false

### 代码

```c++
class Solution {
public:
    bool isLongPressedName(string name, string typed) {
        if (name.size() > typed.size() ||
            (name.size() > 0 && typed.size() == 0) ||
            (name.size() == 0 && typed.size() > 0))
        {
            return false;
        }
        if (name.size() == 0 && typed.size() == 0)
        {
            return true;
        }
        int nameidx = 0;
        int typeidx = 0;
        for (; nameidx < name.size(); ++nameidx)
        {
            bool typefirst = true;
            if (typeidx >= typed.size()) //当两字符串校对过程中，typed先溢出，则返回false
            {
                return false;
            }
            for (; typeidx < typed.size(); ++typeidx)
            {
                if (typefirst == true)
                {
                    if (name[nameidx] == typed[typeidx])
                    {
                        typefirst = false;
                        continue;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    if (name[nameidx] == typed[typeidx])
                    {
                        if (nameidx != name.size() - 1 && name[nameidx] == name[nameidx + 1])
                        {
                            break;
                        }
                        else
                        {
                           continue;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        if (nameidx == name.size())
        {
            return true;
        }
        return false;
    }
};

int main()
{
    Solution so;
//    string name = "alex";string typed = "aaleex";
//    string name = "saeed";string typed = "ssaaedd";
//    string name = "leelee";string typed = "lleeelee";
//    string name = "laiden";string typed = "laiden";
//    string name = "aaaleexxx";string typed = "aaaaleexxxxxxxx";
    string name = "pyplrz";string typed = "ppyypllr"; //这种情况漏掉了
    std::cout << so.isLongPressedName(name, typed) << std::endl;
}
```
