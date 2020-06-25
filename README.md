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
	
	/* отправляем два сообщения */
    	easy_email.send("bbotytch@yandex.ru", u8"Робот", u8"Превед, человег!");
    	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    	easy_email.send("bbotytch@yandex.ru", u8"Робот", u8"Превед, человег! Ещо раз!");
    	return 0;
}
```
