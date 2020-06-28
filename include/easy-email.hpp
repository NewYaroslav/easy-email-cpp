/*
* easy-email-cpp - C ++ library for sending emails
*
* Copyright (c) 2019 Elektro Yar. Email: git.electroyar@gmail.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef EASY_EMAIL_HPP_INCLUDED
#define EASY_EMAIL_HPP_INCLUDED

#include <string>
#include <vector>
#include "curl/curl.h"
#include "base64.h"

class EasyEmail {
private:
    std::string url_mailserver; /**< SMTP сервер для передачи электронной почты */
    std::string email;          /**< Электронная почта */
    std::string password;       /**< Пароль от почты */
    std::string sert_file;      /**< Файл сертификата */
    const uint32_t port = 465;

    bool is_skip_hostname_verification = false;
    bool is_skip_peer_verification = false;

    char error_buffer[CURL_ERROR_SIZE];  /**< Буфер для вывода ошибки при передаче */
    const int TIME_OUT = 60;            /**< Время ожидания ответа */

    bool is_log_visibility = false;

    struct upload_status  {
        size_t lines_read;
        std::string msg;
    };

    static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
        struct upload_status *upload_ctx = (struct upload_status *)userp;
        if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
            return 0;
        }
        size_t len = upload_ctx->msg.length();
        if(upload_ctx->lines_read < len){
            len = std::min(len, size*nmemb);
            memcpy(ptr, upload_ctx->msg.c_str() + upload_ctx->lines_read, len);
            upload_ctx->lines_read += len;
            return len;
        }
        return 0;
    }

    int culr_request(const std::string &toemail, struct upload_status &upload_ctx) {
        struct curl_slist *recipients = NULL;
        CURLcode res = CURLE_OK;
        CURL *curl = curl_easy_init();
        if(curl) {
            /* Установите имя пользователя и пароль */
            curl_easy_setopt(curl, CURLOPT_USERNAME, email.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

            /* Это URL вашего почтового сервера. Обратите внимание
             * на использование smtps: // вместо smtp: //
             * для запроса соединения на основе SSL.
             */
            curl_easy_setopt(curl, CURLOPT_URL, url_mailserver.c_str());
            curl_easy_setopt(curl, CURLOPT_PORT, port);
            curl_easy_setopt(curl, CURLOPT_CAINFO, sert_file.c_str());

            /* Если вы хотите подключиться к сайту, который не использует сертификат,
             * подписанный одним из сертификатов в вашем комплекте CA,
             * вы можете пропустить проверку сертификата сервера.
             * Это делает соединение ОЧЕНЬ МЕНЬШЕ БЕЗОПАСНЫМ.
             *
             * Если у вас есть сертификат CA для сервера, хранящийся где-то еще,
             * чем в комплекте по умолчанию,
             * тогда вам может пригодиться параметр CURLOPT_CAPATH.
             */
            if(is_skip_peer_verification) curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            /* Если сайт, к которому вы подключаетесь, использует другое имя хоста,
             * отличное от того, что они указали в полях commonName (или subjectAltName)
             * сертификата сервера, libcurl откажется подключиться.
             * Вы можете пропустить эту проверку,
             * но это сделает соединение менее безопасным. */
            if(is_skip_hostname_verification) curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            /* Обратите внимание, что эта опция строго не обязательна,
             * ее пропуск приведет к тому, что libcurl отправит команду MAIL FROM
             * с пустыми данными отправителя.
             * Все автоответчики должны иметь пустой обратный путь и должны быть
             * направлены на адрес в обратном пути, который их инициировал.
             * В противном случае они могут вызвать бесконечный цикл.
             * См. RFC 5321 Раздел 4.5.5 для более подробной информации.
             */
            std::string& from = email;
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

            /* Добавьте двух получателей,
             * в данном конкретном случае они соответствуют
             * Кому: и Cc: адресаты в заголовке,
             * но они могут быть любыми получателями.
             */
            recipients = curl_slist_append(recipients, toemail.c_str());
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            /* Мы используем функцию обратного вызова,
             * чтобы указать полезную нагрузку (заголовки и тело сообщения).
             * Вы можете просто использовать опцию CURLOPT_READDATA,
             * чтобы указать указатель FILE для чтения.
             */
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
            curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            /* Поскольку трафик будет зашифрован,
             * очень полезно включить отладочную информацию в libcurl,
             *  чтобы увидеть, что происходит во время передачи
             */
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OUT); // выход через TIME_OUT сек

            /* Отправить сообщение */
            res = curl_easy_perform(curl);

            /* Проверьте на ошибки */
            if(res != CURLE_OK) {
                if(is_log_visibility) {
                    std::cerr << "EasyEmail"<< std::endl;
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                    std::cerr << "Libcurl error in send(), code:\n";
                    std::cerr << "Error: " << error_buffer;
                    std::cerr << std::endl << res << std::endl;
                }
                /* Освободить список получателей */
                curl_slist_free_all(recipients);
                curl_easy_cleanup(curl);
                return res;
            }

            /* Освободить список получателей */
            curl_slist_free_all(recipients);
            curl_easy_cleanup(curl);
        } else return CURL_INIT_ERROR;
        return (int)res;
    }

public:

    enum {
        CURL_INIT_ERROR = -1,
        INVALID_PARAMETER = -2,
    };

    /// Типы параметров
    enum class TypesOptions {
        LOG_VISIBILITY = 0,
        SKIP_PEER_VERIFICATION = 1,
        SKIP_HOSTNAME_VERFICATION = 2,
        EMAIL = 3,
        PASSWORD = 4,
        MAIL_SERVER = 5,
    };

    /** \brief Установить параметры
     * \param opt Имя параметра
     * \param state Состояние
     */
    void set_opt(const TypesOptions opt, const bool state) {
        switch(opt) {
        case TypesOptions::SKIP_PEER_VERIFICATION:
            is_skip_peer_verification = state;
            break;
        case TypesOptions::SKIP_HOSTNAME_VERFICATION:
            is_skip_hostname_verification = state;
            break;
        case TypesOptions::LOG_VISIBILITY:
            is_log_visibility = state;
            break;
        default:
            break;
        }
    }

    /** \brief Установить параметры
     * \param opt Имя параметра
     * \param value Значение
     */
    void set_opt(const TypesOptions opt, const std::string &value) {
        switch(opt) {
        case TypesOptions::EMAIL:
            email = value;
            break;
        case TypesOptions::PASSWORD:
            password =  value;
            break;
        case TypesOptions::MAIL_SERVER:
            url_mailserver = value;
            break;
        default:
            break;
        }
    }

    /** \brief Конструктор класса для отправки email
     */
    EasyEmail() {
        curl_global_init(CURL_GLOBAL_ALL);
        url_mailserver = "smtps://smtp.yandex.ru";
        sert_file = "curl-ca-bundle.crt";
    }

    /** \brief Конструктор класса для отправки email
     * \param _email Потчовый ящик
     * \param _password Пароль от почтового ящика
     */
    EasyEmail(const std::string &_email, const std::string &_password) :
        email(_email), password(_password) {
        curl_global_init(CURL_GLOBAL_ALL);
        url_mailserver = "smtps://smtp.yandex.ru";
        sert_file = "curl-ca-bundle.crt";
    }

    /** \brief Конструктор класса для отправки email
     * \param _url_mailserver Сервис для отправки сообщений
     * \param _email Потчовый ящик
     * \param _password Пароль от почтового ящика
     * \param _sert_file Файл сертификата
     */
    EasyEmail(const std::string &_url_mailserver, const std::string &_email, const std::string &_password, const std::string &_sert_file) :
        url_mailserver(_url_mailserver), email(_email), password(_password), sert_file(_sert_file)  {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~EasyEmail() {

    }

    /** \brief отправить сообщение самому себе
     * \param theme Тема сообщения
     * \param msg Сообщение
     * \return CURLE_OK в случае успеха
     */
    int send(const std::string &theme, const std::string &msg) {
        return send(email, theme, msg);
    }

    /** \brief Отправить текстовый файл на email
     * \param theme Тема сообщения
     * \param msg Сообщение
     * \param file_name Массив имен файлов
     * \param file_content Тело файлов
     * \return CURLE_OK в случае успеха
     */
    int send(const std::string &theme, const std::string &msg, const std::vector<std::string> &file_name, const std::vector<std::string> &file_content) {
        return send(email, theme, msg, file_name, file_content);
    }

    /** \brief Отправить сообщение на email
     * \param toemail Кому отправить сообщение
     * \param theme Тема сообщения
     * \param msg Сообщение
     * \return CURLE_OK в случае успеха
     */
    int send(const std::string &toemail, const std::string &theme, const std::string &msg) {
        struct upload_status upload_ctx;

        upload_ctx.lines_read = 0;
        upload_ctx.msg = "To: <" + toemail + ">\r\n";
        upload_ctx.msg += "From: <" + email + ">\r\n";
        upload_ctx.msg += "Subject: " + theme + "\r\n";
        upload_ctx.msg += "\r\n"; /* пустая строка для разделения заголовков от тела, см. RFC5322 */
        upload_ctx.msg += msg + "\r\n";
        upload_ctx.msg += "\r\n\0";
        return culr_request(toemail, upload_ctx);
    }

    /** \brief Отправить текстовый файл на email
     * \param toemail Кому отправить сообщение
     * \param theme Тема сообщения
     * \param msg Сообщение
     * \param file_name Массив имен файлов
     * \param file_content Тело файлов
     * \return CURLE_OK в случае успеха
     */
    int send(const std::string &toemail, const std::string &theme, const std::string &msg, const std::vector<std::string> &file_name, const std::vector<std::string> &file_content) {
        if(file_name.size() != file_content.size()) return INVALID_PARAMETER;
        struct upload_status upload_ctx;

        upload_ctx.lines_read = 0;
        upload_ctx.msg = "To: <" + toemail + ">\r\n";
        upload_ctx.msg += "From: <" + email + ">\r\n";
        upload_ctx.msg += "Subject: " + theme + "\r\n";
        upload_ctx.msg += "Content-Type: multipart/mixed; boundary=MixedEasyEmailBoundary\r\n";
        upload_ctx.msg += "\r\n"; /* пустая строка для разделения заголовков от тела, см. RFC5322 */
        upload_ctx.msg += "--MixedEasyEmailBoundary\r\n";
        upload_ctx.msg += "Content-Type: multipart/alternative; boundary=AlternativeEasyEmailBoundary\r\n";
        upload_ctx.msg += "\r\n";
        upload_ctx.msg += "--AlternativeEasyEmailBoundary\r\n";
        upload_ctx.msg += "Content-Type: text/plain; charset=\"utf-8\"\r\n";
        upload_ctx.msg += "\r\n";
        upload_ctx.msg += msg + "\r\n";
        upload_ctx.msg += "\r\n";
        upload_ctx.msg += "--AlternativeEasyEmailBoundary\r\n";
        upload_ctx.msg += "Content-Type: multipart/related; boundary=RelatedEasyEmailBoundary\r\n";
        upload_ctx.msg += "\r\n";
        for(size_t i = 0; i < file_name.size(); ++i) {
            std::string file_content_encoded;
            if(!Base64::Encode(file_content[i], &file_content_encoded)) {
                std::cout << "Failed to encode input string" << std::endl;
                return -1;
            }
            upload_ctx.msg += "--RelatedEasyEmailBoundary\r\n";
            upload_ctx.msg += "Content-Type: text/plain; name=\"" + file_name[i] + "\"\r\n";
            upload_ctx.msg += "Content-Transfer-Encoding: base64\r\n";
            upload_ctx.msg += "Content-Disposition: attachment; filename=\"" + file_name[i] + "\"\r\n";
            upload_ctx.msg += "\r\n"; /* пустая строка для разделения заголовков от тела, см. RFC5322 */
            upload_ctx.msg += file_content_encoded + "\r\n";
            upload_ctx.msg += "\r\n"; /* пустая строка для разделения заголовков от тела, см. RFC5322 */
        }
        upload_ctx.msg += "--RelatedEasyEmailBoundary--\r\n";
        upload_ctx.msg += "\r\n";
        upload_ctx.msg += "--AlternativeEasyEmailBoundary--\r\n";
        upload_ctx.msg += "\r\n";
        upload_ctx.msg += "--MixedEasyEmailBoundary--\r\n\0";
        return culr_request(toemail, upload_ctx);
    }
};

#endif // EASY_EMAIL_HPP_INCLUDED
