原题链接：https://leetcode.com/problems/unique-email-addresses/

### 题意

每封电子邮件都包含本地名称和域名，以@符号分隔，例如，在alice@leetcode.com中，alice是本地名称，leetcode.com是域名。除了小写字母，这些电子邮件可能包含'.'或'+'。

1.如果在电子邮件地址的本地名称部分中的某些字符之间添加句点（'.'），则在那里发送的邮件将转发到本地名称中没有点的同一地址。例如，“alice.z@leetcode.com”和“alicez@leetcode.com”转发到同一电子邮件地址。 （请注意，此规则不适用于域名。）

2.如果在本地名称中添加加号（'+'），则会忽略第一个加号后面的所有内容。这允许过滤某些电子邮件，例如m.y+name@email.com将转发到my@email.com。 （同样，此规则不适用于域名。）

给定电子邮件列表，我们会向列表中的每个地址发送一封电子邮件。有多少不同的地址实际接收邮件？

举例:

Input: ["test.email+alex@leetcode.com","test.e.mail+bob.cathy@leetcode.com","testemail+david@lee.tcode.com"]

Output: 2

### 思路

先按规则过滤一遍字符串，过滤完成后，使用unordered_set过滤出重复的字符串，然后输出unordered_set容器的大小，即为结果。

### 代码

```c++
class Solution {
public:
    int numUniqueEmails(vector<string>& emails) {
        if (emails.size() == 0){
            return 0;
        }
        
        unordered_set<string> emailsmap;
        char procedemail[100] = {0};
        int pidx = 0;
        for (auto& email : emails) {
            memset(procedemail, 0, 100);
            pidx = 0;
            bool hasFindAt = false;
            bool hasFindAddPlus = false;
            for (auto& c : email)
            {
                if (hasFindAt == false && c == '@')
                {
                    hasFindAt = true;
                    procedemail[pidx++] = c;
                    continue;
                }
                if (hasFindAt == false && c == '+')
                {
                    hasFindAddPlus = true;
                    continue;
                }
                if (hasFindAt == false && c == '.')
                {
                    continue;
                }
                if (hasFindAt == false && hasFindAddPlus == true)
                {
                    continue;
                }
                procedemail[pidx++] = c;
            }
            string strprocedemail = procedemail;
            emailsmap.insert(strprocedemail);
        }
        return (int)emailsmap.size();
    }
};

int main()
{
    vector<string> emails = {"test.email+alex@leetcode.com","test.e.mail+bob.cathy@leetcode.com","testemail+david@lee.tcode.com", "a.+t+@leetcode.com", ".++@leetcode.com", "+.+.aa@leetcode.com", "@leetcode.com"};
    Solution so;
    std::cout << so.numUniqueEmails(emails) << std::endl;
}
```
