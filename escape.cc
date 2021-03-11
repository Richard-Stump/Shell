#include <iostream>
#include <string>

std::string escape(std::string arg)
{
    std::string str = "";

    for(size_t i = 0; i < arg.length(); i++) {
        if(arg[i] == '\\') {
            str += arg[++i];
        }
        else {
            str += arg[i];
        }
    }

    return str;
}

int main(int argc, char** argv)
{
    while(true) {
        std::string in;
        std::cin >> in;

        std::cout << escape(in) << std::endl;
    }

    return 0;
}