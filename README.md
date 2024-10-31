# README

Данный проект реализует систему управления складом, продуктами и грузовиками. В проекте определены три основных класса: `Product`, `Warehouse`, `Factory` и `Truck`. Ниже представлено описание каждого класса и его методов.
## Компиляция и запуск
- clang++ -std=c++20 -o FGBU main.cpp
- ./FGBU

## Классы и методы

### 1. Класс `Product`

Класс, представляющий продукт.

#### Конструкторы:
- `Product(const std::string& name, double weight, const std::string& packaging, size_t quantity)`:
  Конструктор для инициализации продукта с заданными параметрами.
- `Product()`:
  Конструктор по умолчанию.

#### Члены класса:
- `std::string name`: Название продукта.
- `double weight`: Вес продукта.
- `std::string packaging`: Упаковка продукта.
- `size_t quantity`: Количество продукта.

#### Методы:
- `size_t getQuantity() const`: Возвращает количество продукта.
- `void decreaseQuantity(size_t amount)`: Уменьшает количество продукта на указанное значение.

---

### 2. Класс `Warehouse`

Класс, представляющий склад, на котором хранятся продукты.

#### Конструкторы:
- `Warehouse(const std::string& name, size_t capacity)`:
  Конструктор для инициализации склада с именем и вместимостью.

#### Члены класса:
- `std::string name`: Название склада.
- `size_t capacity`: Вместимость склада.
- `size_t current_load`: Текущая загрузка склада.
- `std::map<std::string, Product> inventory`: Инвентарь склада.
- `std::vector<ArrivalLogEntry> arrival_log`: Журнал поступлений продукции.
- `std::mutex mtx`: Мьютекс для потокобезопасности.
- `bool is_unloading`: Флаг, указывающий, идет ли авторазгрузка.

#### Методы:
- `size_t getFreeSpace() const`: Возвращает количество свободного места на складе.
- `bool storeProduct(const Product& product)`: Добавляет продукт на склад, возвращает `true`, если успешно.
- `std::map<std::string, Product> unload(const std::string& product_name, size_t max_quantity)`:
  Удаляет указанное количество продукта со склада.
- `std::string getName() const`: Возвращает название склада.
- `size_t getProductQuantity(const std::string& product_name) const`: Возвращает количество указанного продукта на складе.
- `void printArrivalLog() const`: Выводит журнал поступлений продукции.
- `bool isOverloaded() const`: Проверяет, перегружен ли склад.
- `void startAutoUnload(std::vector<Truck*>& trucks, const std::string& shop_name)`:
  Запускает автоматическую разгрузку, если склад перегружен.
- `void autoUnload(std::vector<Truck*>& trucks, const std::string& shop_name)`:
  Автоматически разгружает склад, используя доступные грузовики.

#### Потокобезопасная функция авторазгрузки склада

Функция `autoUnload` реализует автоматическую разгрузку склада, когда он перегружен. Она запускается в фоновом режиме в отдельном потоке и является потокобезопасной благодаря использованию механизмов синхронизации (`std::mutex` и `std::lock_guard`).

**Основные этапы работы `autoUnload`:**
1. **Запуск потока**: Функция запускается асинхронно, освобождая основной поток от ожидания окончания разгрузки.
2. **Потокобезопасность**: Для обеспечения безопасности операций с общими ресурсами функция использует блокировки:
    - `std::lock_guard<std::mutex> coutLock` для синхронизации вывода в консоль.
    - `std::unique_lock<std::mutex> lock(mtx)` для блокировки склада на время авторазгрузки.

3. **Сортировка грузовиков**: Грузовики сортируются по текущей загрузке для оптимизации процесса выгрузки.

4. **Процесс разгрузки**: В цикле перебираются грузовики и продукты на складе. Если продукт доступен и в грузовике есть место, продукт выгружается.

---

### 3. Класс `Factory`

Класс, представляющий фабрику, которая производит продукты.

#### Конструкторы:
- `Factory(const std::string& name, double weight, const std::string& packaging, int production_rate)`:
  Конструктор для инициализации фабрики с параметрами.

#### Члены класса:
- `std::string name`: Название фабрики.
- `double weight`: Вес продукции.
- `std::string packaging`: Упаковка продукции.
- `int production_rate`: Темп производства.

#### Методы:
- `void storage(std::vector<Warehouse*>& warehouses)`: Размещает продукцию на складах.
- `Product createProduct()`: Создает продукт на основе параметров фабрики.

---

### 4. Класс `Truck`

Класс, представляющий грузовик, который доставляет продукты.

#### Конструкторы:
- `Truck(const std::string& name, size_t max_capacity)`:
  Конструктор для инициализации грузовика.

#### Члены класса:
- `std::string name`: Название грузовика.
- `size_t max_capacity`: Максимальная грузоподъемность.
- `size_t product_count`: Количество продуктов в грузовике.
- `size_t total_delivered`: Общее количество доставленных продуктов.
- `std::map<std::string, size_t> delivered_products`: Статистика доставленных продуктов.

#### Методы:
- `void loadProduct(const std::string& product_name, size_t count)`: Загружает продукт в грузовик.
- `void unloadProduct(const std::string& shop_name)`: Выгружает продукты в магазин.
- `void deliver(Warehouse* warehouse, const std::string& shop_name, const std::map<std::string, size_t>& requests)`:
  Доставляет продукты из склада в магазин.
- `void printStatistics() const`: Выводит статистику по доставленным продуктам.

---

## Пример использования

В функции `main()` создаются склады, фабрики и грузовики, после чего происходит загрузка складов и автоматическая разгрузка, если это необходимо. Затем выполняется обработка запросов на доставку.

```cpp

int main() {
    // Создаем склады с названиями и вместимостью
    Warehouse warehouseA("Склад A", 100);
    Warehouse warehouseB("Склад B", 100);
    std::vector<Warehouse*> warehouses = { &warehouseA, &warehouseB };

    // Создаем грузовики
    Truck truck("Грузовик 1", 10);
    Truck truck2("Грузовик 2", 8);
    std::vector<Truck*> trucks = { &truck, &truck2 };

    std::cout << "\n---ЗАГРУСКА СКЛАДОВ---\n\n";

    // Создаем заводы и наполняем склады
    Factory factory1("Продукт A", 10.0, "Коробка", 90);
    Factory factory2("Продукт 1", 10.0, "Коробка", 90);
    Factory factory3("Продукт A", 10.0, "Коробка", 90);

    factory1.storage(warehouses);
    factory2.storage(warehouses);
    factory3.storage(warehouses);

    // Добавляем паузу для просмотра состояния складов перед авторазгрузкой
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\nЗапуск авторазгрузки для складов:\n";
    for (auto* warehouse : warehouses) {
        warehouse->startAutoUnload(trucks, "Магазин 1"); // Запуск авторазгрузки при необходимости
    }

    // Добавляем паузу для наблюдения за авторазгрузкой в фоне
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\n---------СОЗДАНИЕ И ОБРАБОТКА ЗАПРОСА НА ДОСТАВКУ-----------\n\n";
    std::map<std::string, size_t> requests1 = {
            {"Продукт A", 10},
            {"Продукт 1", 12}
    };

    // Обрабатываем доставку
    truck.deliver(warehouses, "Магазин 1", requests1);

    // Добавляем паузу для завершения предыдущих процессов доставки
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n-------ЖУРНАЛ ПОСТУПЛЕНИЙ-------\n\n";
    warehouseA.printArrivalLog();
    warehouseB.printArrivalLog();

    std::cout << "\n---------СТАТИСТИКА ГРУЗОВИКОВ----------\n\n";
    truck.printStatistics();
    truck2.printStatistics();

    // Небольшая пауза перед завершением программы, чтобы убедиться, что все процессы завершены
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}

