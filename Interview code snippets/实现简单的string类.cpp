#include <iostream>
using namespace std;

class String
{
public:
    //构造函数
    String();
    String(const char* str);
    
    //赋值构造函数
    String(const String& str);
    
    //赋值运算符函数
    String& operator=(const String& str);
    
    //赋值运算符函数-右值引用
    String& operator=(String&& str);
    
    //是否相等
    bool operator==(const String& str);
    
    //+运算符
    String operator+(const String& str);
    
    //析构函数
    virtual ~String();
    
    //获取长度
    size_t GetLength();
    
    void show(){cout << _pdata <<endl;};
private:
    size_t _len;
    char* _pdata;
};

String::String()
{
    _len = 0;
    _pdata = nullptr;
}

String::String(const char* str)
{
    if (str == nullptr)
    {
        _pdata = new char[1];
        _pdata[0] = '\0';
        _len = 0;
    }
    else
    {
        _len = strlen(str);
        _pdata = new char[_len + 1];
        memcpy(_pdata, str, _len);
        _pdata[_len] = '\0';

    }
}

String::String(const String& str)
{
    _len = str._len;
    _pdata = new char[_len + 1];
    memcpy(_pdata, str._pdata, _len);
    _pdata[_len] = '\0';
}

String& String::operator=(String&& str)
{
    if (_pdata)
    {
        delete [] _pdata;
        _pdata = nullptr;
        _len = 0;
    }
    _pdata = str._pdata;
    _len = str._len;
    str._len = 0;
    str._pdata = nullptr;
    return *this;
}

String& String::operator=(const String& str)
{
    if (&str != this)
    {
        if (_len > str._len)
        {
            memset(_pdata, 0, _len);
            _len = 0;
            memcpy(_pdata, str._pdata, str._len);
            _len = str._len;
            _pdata[_len] = '\0';
        }
        else if (_len == str._len)
        {
            memcpy(_pdata, str._pdata, str._len);
        }
        else
        {
            delete [] _pdata;
            _pdata = nullptr;
            
            _len = str._len;
            _pdata = new char[_len + 1];
            memcpy(_pdata, str._pdata, _len);
            _pdata[_len] = '\0';
        }
    }
    return *this;
}

bool String::operator==(const String& str)
{
    //判断相等函数，返回值只有true和false，先判断长度是否一致，再判断内容是否一致
    if (_len != str._len)
    {
        return false;
    }
    else
    {
        return strcmp(_pdata, str._pdata) == 0 ? true : false;
    }
}

String String::operator+(const String& str)
{
    String newString;
    if (str._pdata == nullptr)
    {
        newString = *this;
    }
    else if (_pdata == nullptr)
    {
        newString = str;
    }
    else
    {
        newString._pdata = new char[_len + str._len + 1];
        strcpy(newString._pdata, _pdata);
        strcat(newString._pdata, str._pdata);
    }
    return newString;
}

String::~String()
{
    if (_pdata != nullptr)
    {
        delete [] _pdata;
        _pdata = nullptr;
        _len = 0;
    }
}

int main()
{
    //测试赋值运算符函数返回对象引用的必要性
    String str1("abc");
    String str2("123");
    String str3("456");
    str1.show();
    str2.show();
    str3.show();
    str3 = str1 = str2;//str3.operator=(str1.operator=(str2))
    //    (str3 = str1) = str2;
    str3.show();
    str1.show();
    
    //测试右值引用节约一次内存申请释放的额外操作
    String str4;
    str4 = String("098");
    
    String str5 = str1 + str2;
    str5.show();
    
    cout << (str1 == str2) << endl;
    cout << (str5 == str1) << endl;
    return 0;
}
