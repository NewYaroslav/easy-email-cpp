# easy-email-cpp
C++ библиотека для отправки сообщений на email


## Описание
Данная *header-only* библиотека позволяет отправлять сообщения на *email*, использует *curl* и написана на *C++11*.

Пример кода:

```cpp
#include <iostream>
#include "easy-email.hpp"
#include <thread> // только для std::this_thread::sleep_for

int main() {
	std::cout << "Hello Email!" << std::endl;
	
	std::string email("example@yandex.ru");
	std::string pass("123456");
	
	EasyEmail easy_email(email, pass);
	
	/* отправляем два сообщения на почтовый ящик to_two_example@yandex.ru */
	easy_email.send("to_two_example@yandex.ru", u8"Робот", u8"Превед, человег!");
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	easy_email.send("to_two_example@yandex.ru", u8"Робот", u8"Превед, человег! Ещо раз!");
	
	/* отправляем сообщение на почтовый ящик example@yandex.ru */
	easy_email.send(u8"Робот", u8"Превед сам себе!");
	return 0;
}
```

Также можно отправлять текстовые файлы:

```cpp
		easy_email.send(email, u8"Файл самому себе!", u8"Вот файлы",
        {"Пример.txt", "Пример2.txt"},
        {u8"Вот мы и добрались до самого сокровеного...", 
		 u8"Вот мы и добрались до самого сокровеного... Наверно..."});
		
		//
		easy_email.send(u8"Файл самому себе 2.0", u8"Вот файлы еще раз",
        {"Пример3.txt", "Пример4.txt"},
        {u8"Вот мы и добрались до самого сокровеного... Или нет...", 
		 u8"Вот мы и добрались до самого сокровеного... Наверно... Хотя.."});
```

**Не забудьте про файл сертификата *curl-ca-bundle.crt*! Он должен быть в папке с программой! Или можно указать путь к другому вашему файлу сертификата через конструктор класса или параметры класса.**
