原题链接：https://leetcode.com/problems/keyboard-row/

### 题意

给定一个字符串数组，要求返回数组中“仅仅使用美国键盘的一行就能敲出来”的字符串。

输入输出举例:

Input: ["Hello", "Alaska", "Dad", "Peace"]

Output: ["Alaska", "Dad"]

你可以假设以下条件成立：可以多次使用键盘上的字母；你可以假设输入的字符串仅仅包含字母表中的字母。

### 思路

构造三个hash表，每个表保存键盘的一行，然后两层循环，内层循环第一次判断第一个字母属于哪一行，然后从第二个字母开始进行行校验，如果一直校验到字符串末尾都符合条件，则添加到返回值中，反之则剔除。

边界考虑：空字符串，大小为1的字符串

### 代码

```c++
class Solution {
public:
    vector<string> findWords(vector<string>& words) {
        vector<string> ret;
        if (words.size() == 0)
        {
            return ret;
        }
        
        int line1[256] = {0};
        line1['q'] = 1;line1['Q'] = 1;
        line1['w'] = 1;line1['W'] = 1;
        line1['e'] = 1;line1['E'] = 1;
        line1['r'] = 1;line1['R'] = 1;
        line1['t'] = 1;line1['T'] = 1;
        line1['y'] = 1;line1['Y'] = 1;
        line1['u'] = 1;line1['U'] = 1;
        line1['i'] = 1;line1['I'] = 1;
        line1['o'] = 1;line1['O'] = 1;
        line1['p'] = 1;line1['P'] = 1;
        
        int line2[256] = {0};
        line2['a'] = 1;line2['A'] = 1;
        line2['s'] = 1;line2['S'] = 1;
        line2['d'] = 1;line2['D'] = 1;
        line2['f'] = 1;line2['F'] = 1;
        line2['g'] = 1;line2['G'] = 1;
        line2['h'] = 1;line2['H'] = 1;
        line2['j'] = 1;line2['J'] = 1;
        line2['k'] = 1;line2['K'] = 1;
        line2['l'] = 1;line2['L'] = 1;
        
        int line3[256] = {0};
        line3['z'] = 1;line3['Z'] = 1;
        line3['x'] = 1;line3['X'] = 1;
        line3['c'] = 1;line3['C'] = 1;
        line3['v'] = 1;line3['V'] = 1;
        line3['b'] = 1;line3['B'] = 1;
        line3['n'] = 1;line3['N'] = 1;
        line3['m'] = 1;line3['M'] = 1;
        
        for (auto& word: words)
        {
            if (word.length() == 0)
            {
                continue;
            }
            if (word.length() == 1)
            {
                ret.push_back(word);
                continue;
            }
            
            int tag = 0;
            int i = 0;
            for (; i < word.length(); ++i)
            {
                if (i == 0)
                {
                    if (line1[word[i]] == 1)
                    {
                        tag = 1;
                    }
                    else if (line2[word[i]] == 1)
                    {
                        tag = 2;
                    }
                    else if (line3[word[i]] == 1)
                    {
                        tag = 3;
                    }
                }
                else
                {
                    if (tag == 1 && line1[word[i]] == 1)
                    {
                        continue;
                    }
                    else if (tag == 2 && line2[word[i]] == 1)
                    {
                        continue;
                    }
                    else if (tag == 3 && line3[word[i]] == 1)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if (i == word.size())
            {
                ret.push_back(word);
            }
        }
        return ret;
    }
};
```
