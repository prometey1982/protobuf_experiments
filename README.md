# Клиентская часть системы запроса данных о проектах и логах

## Описание

C++ клиент для взаимодействия с сервером через WebSocket соединение с TLS шифрованием. Клиент реализует функционал согласно техническому заданию:

- Запрос доступных проектов по VIN
- Запрос проекта по VIN и имени проекта
- Загрузка прошивок на сервер
- Загрузка логов на сервер

## Требования

- C++20 компилятор
- CMake 3.16+
- Boost 1.70+
- OpenSSL
- Zlib
- Protobuf

## Сборка

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Структура проекта

```
src/
├── main.cpp                 # Основная программа
├── websocket/
│   ├── client.hpp          # WebSocket клиент
│   └── client.cpp
├── protobuf/
│   ├── message_builder.hpp # Построитель protobuf сообщений
│   └── message_builder.cpp
├── project/
│   ├── project_manager.hpp # Менеджер проектов
│   └── project_manager.cpp
└── client/
    ├── client.hpp          # Основной клиентский класс
    └── client.cpp
```

## Использование

```cpp
#include "client/client.hpp"

int main() {
    // Создание клиента
    Client client("localhost", "8080", "/ws", "./projects");
    
    // Установка callback функций
    client.set_available_projects_callback([](const std::string& vin, const std::vector<std::string>& projects) {
        // Обработка доступных проектов
    });

    // Подключение
    client.connect();

    // Запрос доступных проектов
    client.request_available_projects("ABC123XYZ456789");

    // Запрос проекта
    client.request_project("ABC123XYZ456789", "project1");

    // Загрузка логов
    std::vector<uint8_t> log_data = {0x10, 0x20, 0x30};
    client.upload_logs("ABC123XYZ456789", "log1", log_data);

    // Загрузка прошивки
    std::vector<uint8_t> flash_data = {0x40, 0x50, 0x60};
    client.upload_flash("ABC123XYZ456789", "flash1", flash_data);
    
    return 0;
}
```

## Особенности реализации

- **Асинхронные операции**: Все обращения к серверу выполняются асинхронно
- **TLS соединение**: Используется защищенное WebSocket соединение
- **Локальное хранилище**: Проекты сохраняются локально на диске
- **Проверка CRC32**: Контрольные суммы проектов проверяются для избежания повторной загрузки
- **Callback система**: Уведомления о событиях через callback функции

## Конфигурация

- Хранилище проектов: файлы на диске в указанной директории
- Формат сообщений: Protobuf (определен в `proto/interface.proto`)
- Транспорт: WebSocket с TLS
- Библиотека: Boost.Beast

## Зависимости

- Boost.Beast для WebSocket
- Boost.Asio для асинхронных операций
- OpenSSL для TLS
- Protobuf для сериализации сообщений
