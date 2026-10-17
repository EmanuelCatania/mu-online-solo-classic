# MU Online Solo Classic (0.97k) - WebAssembly Server & Engine

> Повноцінний рушій сервера **MU Online 0.97k**, портований на **WebAssembly (WASM)** за допомогою **Emscripten** для браузерної соло-гри.

## 🎯 Концепція проєкту

Класичний сервер MU Online був розподіленою системою з 4-х мережевих демонів (`ConnectServer`, `JoinServer`, `DataServer`, `GameServer`), які вимагали реляційної бази даних MySQL/MSSQL і постійних TCP-з'єднань.

Для соло-гри ця архітектура перетворена на **компактний монолітний WASM-рушій**:
1. **ConnectServer і JoinServer усунуто**: Авторизація відбувається автоматично всередині локальної гри.
2. **DataServer / MySQL замінено на локальне сховище (SoloDataServer)**: Персонажі, інвентар та склад зберігаються локально (у файловій системі `IDBFS` / IndexedDB браузера) без зовнішньої БД.
3. **Сокети замінено на In-Memory Packet Channel (SoloTransport)**: Клієнт і сервер обмінюються бінарними пакетами C1/C2 через пам'ять або `Web Worker` з нульовою затримкою (0 ms latency).
4. **Усі оригінальні карти та конфіги (0.97k)** запаковані у віртуальну файлову систему (`/Data`):
   - 17 карт і зон колізій (`Terrain1.att` ... `Terrain17.att` — Lorencia, Devias, Noria, Lost Tower, Atlans, Tarkan, Blood Castle, Devil Square).
   - 3,245 монстрів і спавнів (`Monster.txt`, `MonsterSetBase.txt`).
   - Усі формули бою, дропу, прокачки, квестів та Chaos Machine.

---

## 📁 Структура проєкту

```
mu-online-solo-classic/
├── src/
│   └── server/
│       ├── Common/
│       │   └── Platform.h          # Емуляція Windows API та типів під POSIX/WASM
│       ├── GameServer/             # Оригінальна ігрова логіка (бойовка, скіли, монстри)
│       ├── Solo/
│       │   ├── SoloServer.h        # C API експорту під WebAssembly
│       │   ├── SoloServerMain.cpp  # Життєвий цикл сервера, ініціалізація та Tick
│       │   ├── SoloTransport.h     # Інтерфейс доставки пакетів клієнту
│       │   ├── SoloSocketManager.cpp # In-memory сокет-менеджер
│       │   ├── SoloSocketManagerUdp.cpp # Заглушка UDP
│       │   ├── SoloConnection.cpp  # Віртуальні з'єднання Join/DataServer
│       │   └── SoloDataServer.cpp  # Локальне збереження персонажів і складу
│       ├── Data/                   # Всі таблиці, конфіги та .att карти
│       ├── CMakeLists.txt          # CMake скрипт під Emscripten
│       └── test_node.cjs           # Автономний Node.js тест сервера
├── web/
│   ├── index.html                  # Інтерактивна панель моніторингу та тестування
│   ├── server_worker.js            # Web Worker для запуску WASM-сервера у фоні
│   ├── serve.cjs                   # Локальний HTTP сервер
│   ├── muserver.wasm               # Зібраний бінарний WASM (698 KB)
│   ├── muserver.js                 # JS завантажувач
│   └── muserver.data               # Запаковані ігрові ассети (1.47 MB)
└── reference/
    └── Mu-Linux-0.97k/             # Еталонний C++ вихідний код
```

---

## 🛠️ Збірка сервера (WebAssembly)

### Вимоги:
- [Emscripten SDK (emsdk)](https://emscripten.org/)
- CMake & Ninja

### Команда збірки:
```bash
# 1. Активувати середовище Emscripten
call C:\Users\<user>\emsdk\emsdk_env.bat

# 2. Налаштувати CMake
emcmake cmake -S src/server -B src/server/build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 3. Збірка
ninja -C src/server/build

# 4. Скопіювати зібрані артефакти до web/
copy src\server\build\muserver.* web\
```

---

## 🚀 Запуск тестового веб-стенду

Для перевірки роботи сервера у Web Worker з живим інспектором пакетів:

```bash
node web/serve.cjs
```
Відкрийте у браузері: **http://127.0.0.1:8088**

У тестовій панелі доступні:
- **Метрики:** кількість заспавнених монстрів (3,245), активні гравці, частота оновлення (20 TPS).
- **Покроковий хендшейк:**
  1. Підключення сесії гравця (`Connect Player`).
  2. Авторизація акаунту (`Login`).
  3. Отримання списку персонажів (`Char List`).
  4. Створення персонажа (`Create Dark Knight`).
  5. Вхід у світ Lorencia (`Select HeroDK`).
  6. Рух та атака по монстрах.
- **Live Packet Inspector:** розбір усіх вхідних і вихідних пакетів MU (C1/C2) у реальному часі.
- **Термінал логів:** системні повідомлення GameServer.
