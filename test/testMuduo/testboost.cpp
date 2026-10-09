#include <iostream>
#include <boost/bind.hpp>
#include <string>

class Hello {
public:
    void say(std::string name) {
        std::cout << name << " say: hello world!\n";
    }
};

int main() {
    Hello hello;

    auto function =
        boost::bind(&Hello::say, &hello, "Tom");

    function();
}