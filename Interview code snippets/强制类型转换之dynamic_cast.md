参考自：https://blog.csdn.net/ydar95/article/details/69822540

### dynamic_cast

dynamic_cast作为最重要的强制类型转换运算符，它在对象的多态性展示上起着重要的作用。

看第一段代码：基类指针是指向子类对象的一个指针，将基指针强制转为基类指针、基类指针转为子类指针、子类指针又转为基类指针，我们可以看到这种转换都是能成功，且没有任何运行异常，能达到预期效果（**所有调用都发生了动态绑定，运行的是子类的print函数** ）

看第二段代码：基类指针是指向基类对象的一个指针，把该指针转为子类指针时，没有任何报错，dynamic_cast返回了**nullptr**，这足以说明dynamic_cast在运行期间对指针指向的对象空间进行了**“运行期类型信息”**，它查阅了该指针指向的内存空间的虚函数表信息，发现没有任何一个虚函数被子类的虚函数地址复写，因此它可以认为该指针指向的是一个基类对象，所以给出了转换失败返回nullptr。

```c++
#include<iostream>
using namespace std;

class Base{
public:
    Base() {}
    virtual ~Base() {std::cout << "~Base()" << endl; };
    virtual void print() {
        std::cout << "I'm Base" << endl;
    }
};

class Sub: public Base{
public:
    Sub() {}
    virtual ~Sub() { std::cout << "~Sub()" << endl; };
    virtual void print() {
        std::cout << "I'm Sub" << endl;
    }
};

int main() {
    
    {
        Base * base_ptr = new Sub();
        base_ptr->print();
        
        //基类指针强制转为基类指针，动态绑定依然是子类的函数
        Base* base2base = dynamic_cast<Base*>(base_ptr);
        if (base2base != nullptr) {
            base2base->print();
        }
        cout << "base2base val is: " << base2base << endl;
        
        //基类指针强制转为子类指针，动态绑定依然是子类的函数
        Sub* base2sub = dynamic_cast<Sub*>(base2base);
        if (base2sub != nullptr) {
            base2sub->print();
        }
        cout << "base2sub val is: " << base2sub << endl;
        
        //子类指针强制转为基类指针，动态绑定依然是子类的函数
        Base* sub2base = dynamic_cast<Base*>(base2sub);
        if (sub2base != nullptr) {
            sub2base->print();
        }
        cout << "sub2base val is: " << sub2base << endl;
        cout << endl;
        delete base_ptr;
    }

    {
        Base *base = new Base();
        base->print();
        Sub  *base2sub = dynamic_cast<Sub*>(base);
        if (base2sub != nullptr) {
            base2sub->print();
        }
        cout <<" base2sub val is: "<< base2sub << endl;
        delete base;
    }
    return 0;
}
```

输出：

**I'm Sub**

**I'm Sub**

**base2base val is: 0x101838f00**

**I'm Sub**

**base2sub val is: 0x101838f00**

**I'm Sub**

**sub2base val is: 0x101838f00**



**~Sub()**

**~Base()**

**I'm Base**

 **base2sub val is: 0x0**

**~Base()**
