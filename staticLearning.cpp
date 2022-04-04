#include <iostream>
#include <string>

using namespace std;

class test
{
private:
    static int m_value1;

public:
    test()
    {
        m_value1++;
        m_value2++;
    }
    static int m_value2;
    int getValue()
    {
        return m_value1;
    }
};
//需要在类外分配内存空间、赋值
int test::m_value2 = 2;
int test::m_value1 = 3; //可以直接修改类中private成员

int main()
{
    test t = test();
    cout << t.getValue();
}