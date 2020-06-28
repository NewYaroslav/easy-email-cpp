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
    easy_email.send(u8"Робот", u8"Превед самому себе!");
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    easy_email.send(email, u8"Файл самому себе!", u8"Вот файлы",
        {"Пример.txt", "Пример2.txt"},
        {u8"Вот мы и добрались до самого сокровеного...", u8"Вот мы и добрались до самого сокровеного... Наверно..."});
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    easy_email.send(u8"Файл самому себе 2.0", u8"Вот файлы еще раз",
        {"Пример3.txt", "Пример4.txt"},
        {u8"Вот мы и добрались до самого сокровеного... Или нет...", u8"Вот мы и добрались до самого сокровеного... Наверно... Хотя.."});
    return 0;
}
