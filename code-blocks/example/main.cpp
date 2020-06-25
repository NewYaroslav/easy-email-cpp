#include <iostream>
#include "easy-email.hpp"
#include <thread>

int main() {
    std::cout << "Hello Email!" << std::endl;
    std::string email("example@yandex.ru");
    std::string pass("12345");
    EasyEmail easy_email(email, pass);
    easy_email.send("to_two_example@yandex.ru", u8"Робот", u8"Превед, человег!");
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    easy_email.send("to_two_example@yandex.ru", u8"Робот", u8"Превед, человег! Ещо раз!");
    return 0;
}
