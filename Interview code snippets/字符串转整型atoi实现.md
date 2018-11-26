### atoi实现

实现思路：

异常输入考虑：

- 考虑输入为空指针、字符串大小为0
- 考虑正数、负数的情况
- 考虑异常字符的输出提示
- 考虑字符串超出int承载范围的输入情况

```c++
#include <math.h>
#include <limits.h>
#include <iostream>
class Solution
{
public:
    bool _isvalid = false;
    int my_atoi(const char* str)
    {
        if (str == nullptr)
        {
            _isvalid = false;
            return 0;
        }
        auto len = strlen(str);
        if(len <= 0)
        {
            _isvalid = false;
            return 0;
        }
        
        long long ret = 0; //累加过程中，可能出现大于int的值
        if (str[0] == '-')
        {
            for (int i = 1; i < len; ++i)
            {
                if (str[i] >= '0' && str[i] <= '9')
                {
                    ret = ret * 10 - (str[i]  - '0');
                    if (ret < INT_MIN)
                    {
                        _isvalid = false;
                        return 0;
                    }
                }
                else
                {
                    _isvalid = false;
                    return 0;
                }
            }
        }
        else if (str[0] == '+' || (str[0] >= '0' && str[0] <= '9'))
        {
            for (int i = 0; i < len; ++i)
            {
                if (str[i] >= '0' && str[i] <= '9')
                {
                    ret = ret * 10 + (str[i]  - '0');
                    if (ret > INT_MAX)
                    {
                        _isvalid = false;
                        return 0;
                    }
                }
                else
                {
                    _isvalid = false;
                    return 0;
                }
            }
        }
        else
        {
            _isvalid = true;
            return 0;
        }
        _isvalid = true;
        return (int)ret;
    }
};


int main()
{
    Solution so;
    std::cout << so.my_atoi("387232") << " valid="<< so._isvalid << std::endl;
    std::cout << so.my_atoi("+3872123987") << " valid="<< so._isvalid << std::endl;
    std::cout << so.my_atoi("a872123987") << " valid="<< so._isvalid << std::endl;
    std::cout << so.my_atoi("2147483647") << " valid="<< so._isvalid << std::endl; //最大值 0x7FFFFFFF
    std::cout << so.my_atoi("2147483648") << " valid="<< so._isvalid << std::endl;//最大值加1

    std::cout << so.my_atoi("-111233") << " valid="<< so._isvalid << std::endl;
    std::cout << so.my_atoi("-asd11") << " valid="<< so._isvalid << std::endl;
    std::cout << so.my_atoi("-2147483648") << " valid="<< so._isvalid << std::endl; //最小值 0x80000000
    std::cout << so.my_atoi("-2147483649") << " valid=" << so._isvalid << std::endl;//最小值减1
}
```

