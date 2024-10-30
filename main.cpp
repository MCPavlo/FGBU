#include "classes.h"

// Product implementations
Product::Product(const std::string& name, double weight, const std::string& packaging, size_t quantity)
        : name(name), weight(weight), packaging(packaging), quantity(quantity) {}

Product::Product() : name(""), weight(0), packaging(""), quantity(0) {}

// Warehouse implementations
Warehouse::Warehouse(const std::string& name, size_t capacity)
        : name(name), capacity(capacity), current_load(0) {}

size_t Warehouse::getFreeSpace() const {
    return capacity - current_load;
}

bool Warehouse::storeProduct(const Product& product) {
    size_t free_space = getFreeSpace();
    if (product.quantity > free_space) {
        std::cout << "Предупреждение: недостаточно места для продукта " << product.name
                  << " на складе " << name << ". Запрашиваемое количество: " << product.quantity
                  << ", доступно: " << free_space << "\n";
        return false;
    }
    current_load += product.quantity;
    inventory[product.name] = product;
    recordArrival(product); // Записываем поступление продукции
    std::cout << "Продукция добавлена на склад " << name << ": " << product.name
              << " - " << product.quantity << " ед.\n";
    return true;
}

std::map<std::string, Product> Warehouse::unload(const std::string& product_name, size_t max_quantity) {
    std::map<std::string, Product> load;
    size_t total_units = 0;
    auto it = inventory.find(product_name);
    if (it != inventory.end()) {
        size_t quantity_to_take = std::min(it->second.quantity, max_quantity);
        if (quantity_to_take > 0) {
            load[product_name] = Product(product_name, it->second.weight, it->second.packaging, quantity_to_take);
            it->second.quantity -= quantity_to_take; // Уменьшаем количество
            current_load -= quantity_to_take; // Уменьшаем текущую загрузку
            total_units += quantity_to_take;
        }
    }
    std::cout << "Склад" <<" " <<name << " "<<"отгружен на " << total_units << " ед. продукта " << product_name << ".\n";
    return load;
}

std::string Warehouse::getName() const {
    return name;
}

size_t Warehouse::getProductQuantity(const std::string& product_name) const {
    auto it = inventory.find(product_name);
    return (it != inventory.end()) ? it->second.quantity : 0;
}

void Warehouse::printArrivalLog() const {
    std::cout << "Журнал поступления продукции на склад " << name << ":\n";
    for (const auto& entry : arrival_log) {
        std::cout << "Фабрика: " << entry.factory_name << ", Продукт: " << entry.product_name
                  << ", Количество: " << entry.quantity << "\n";
    }
}

bool Warehouse::isOverloaded() const {
    double fill_percentage = static_cast<double>(current_load) / capacity * 100;
    if (fill_percentage >= 95.0) {
        std::cout << "Склад " << name << " загружен на " << fill_percentage << "% или более.\n";
        return true;
    }
    return false;
}

void Warehouse::autoUnload(std::vector<Truck*>& trucks, const std::string& shop_name) {
    for (auto& truck : trucks) {
        // Проверка, перегружен ли склад
        if (!isOverloaded()) {
            break; // Если не перегружен, останавливаем авторазгрузку
        }

        // Цикл по всем продуктам на складе
        for (auto& productEntry : inventory) {
            Product& product = productEntry.second;
            size_t availableAmount = product.getQuantity(); // Получаем доступное количество продукта
            size_t spaceInTruck = truck->getCapacity() - truck->getCurrentLoad(); // Свободное место в грузовике

            if (availableAmount == 0 || spaceInTruck == 0) {
                continue; // Пропускаем продукт, если его нет или грузовик заполнен
            }

            // Вычисляем, сколько можем отгрузить
            size_t unloadAmount = std::min(availableAmount, spaceInTruck);

            // Если количество для выгрузки — 0, переходим к следующему продукту или грузовику
            if (unloadAmount == 0) {
                continue;
            }

            // Выгружаем продукцию
            product.decreaseQuantity(unloadAmount);  // Уменьшаем количество продукта на складе
            current_load -= unloadAmount; // Уменьшаем текущую загрузку склада
            truck->addProduct(product.getName(), unloadAmount); // Добавляем продукт в грузовик

            std::cout << "Склад отгружен на " << unloadAmount << " ед. продукта " << product.getName() << ".\n";

            // Теперь сразу же выгружаем в магазин
            truck->unloadProduct(shop_name);

            // Проверка, нужно ли дальше разгружать
            if (!isOverloaded()) {
                return; // Останавливаем авторазгрузку, если склад больше не перегружен
            }
        }
    }
}


void Warehouse::recordArrival(const Product& product) {
    arrival_log.emplace_back("Фабрика", product.name, product.quantity); // Записываем поступление
}

// Factory implementations
Factory::Factory(const std::string& name, double weight, const std::string& packaging, int production_rate)
        : name(name), weight(weight), packaging(packaging), production_rate(production_rate) {}

void Factory::storage(std::vector<Warehouse*>& warehouses) {
    Product product = createProduct();
    size_t remaining_quantity = product.quantity;

    // Сначала пытаемся найти склад, который может вместить весь продукт
    for (auto& warehouse : warehouses) {
        if (warehouse->getFreeSpace() >= remaining_quantity) {
            if (warehouse->storeProduct(product)) {
                std::cout << "Продукт " << product.name << " полностью размещен на складе " << warehouse->getName() << "\n";
                return; // Продукт успешно размещен
            }
        }
    }

    // Если не вмещается, распределяем по частям
    for (auto& warehouse : warehouses) {
        if (remaining_quantity == 0) {
            break;
        }

        size_t free_space = warehouse->getFreeSpace();
        if (free_space > 0) {
            size_t quantity_to_store = std::min(remaining_quantity, free_space);
            Product partial_product = Product(product.name, product.weight, product.packaging, quantity_to_store);
            if (warehouse->storeProduct(partial_product)) {
                remaining_quantity -= quantity_to_store;
            }
        }
    }

    if (remaining_quantity > 0) {
        std::cout << "Не удалось сохранить всю продукцию " << product.name
                  << ": остаток " << remaining_quantity << " ед.\n";
    }
}

Product Factory::createProduct() {
    return Product(name, weight, packaging, production_rate);
}

// Truck implementations
Truck::Truck(const std::string& name, size_t max_capacity)
        : name(name), max_capacity(max_capacity), product_count(0), total_delivered(0) {}

void Truck::loadProduct(const std::string& product_name, size_t count) {
    if (product_count + count <= max_capacity) {
        product_count += count;
        std::cout << "Загружено " << count << " ед. продукта " << product_name << " в грузовик " << name << ".\n";
    } else {
        std::cout << "Ошибка: не хватает места в грузовике " << name << " для загрузки " << count << " ед. продукта " << product_name << ".\n";
    }
}

void Truck::unloadProduct(const std::string& shop_name) {
    // Логика выгрузки в магазин
    std::cout << "Грузовик " << name << " выгружает продукцию в магазин " << shop_name << ".\n";
    product_count = 0; // После выгрузки грузовик пуст
}

void Truck::deliver(Warehouse* warehouse, const std::string& shop_name, const std::map<std::string, size_t>& requests) {
    // Логика доставки из склада в магазин
    for (const auto& request : requests) {
        const std::string& product_name = request.first;
        size_t quantity = request.second;

        // Выгружаем продукт из склада
        auto unloaded = warehouse->unload(product_name, quantity);
        total_delivered += unloaded[product_name].quantity;
        delivered_products[product_name] += unloaded[product_name].quantity;
    }
    unloadProduct(shop_name); // После доставки, выгружаем в магазин
}

void Truck::deliver(const std::vector<Warehouse*>& warehouses, const std::string& shop_name, const std::map<std::string, size_t>& requests) {
    // Проверка возможности загрузки полного заказа в один склад
    for (auto warehouse : warehouses) {
        bool can_fulfill_order = true;
        for (const auto& request : requests) {
            const std::string& product_name = request.first;
            size_t required_quantity = request.second;

            if (warehouse->getProductQuantity(product_name) < required_quantity) {
                can_fulfill_order = false;
                break; // Не хватает количества, переходим к следующему складу
            }
        }

        // Если весь заказ может быть выполнен с одного склада
        if (can_fulfill_order) {
            for (const auto& request : requests) {
                const std::string& product_name = request.first;
                size_t required_quantity = request.second;
                auto unloaded = warehouse->unload(product_name, required_quantity);
                total_delivered += unloaded[product_name].quantity;
                delivered_products[product_name] += unloaded[product_name].quantity;
            }
            unloadProduct(shop_name); // Выгружаем все сразу в магазин
            return; // Завершаем, так как весь заказ выполнен с одного склада
        }
    }

    // Если один склад не может полностью удовлетворить заказ, распределяем по нескольким складам
    bool product_found = false; // Флаг для проверки наличия продуктов

    for (const auto& request : requests) {
        const std::string& product_name = request.first;
        size_t required_quantity = request.second;
        size_t remaining_quantity = required_quantity;

        for (auto warehouse : warehouses) {
            size_t available_quantity = warehouse->getProductQuantity(product_name);
            if (available_quantity > 0) {
                product_found = true; // Отмечаем, что продукт найден
                size_t quantity_to_unload = std::min(available_quantity, remaining_quantity);
                auto unloaded = warehouse->unload(product_name, quantity_to_unload);
                total_delivered += unloaded[product_name].quantity;
                delivered_products[product_name] += unloaded[product_name].quantity;
                remaining_quantity -= quantity_to_unload;

                if (remaining_quantity == 0) {
                    break; // Переходим к следующему продукту, так как количество полностью загружено
                }
            }
        }

        if (remaining_quantity > 0) {
            std::cout << "Продукт " << product_name << " недоступен в необходимом количестве (" << required_quantity << " ед.) на складах.\n";
        }
    }

    if (product_found) {
        unloadProduct(shop_name); // Если хоть один продукт загружен, выгружаем в магазин
    } else {
        std::cout << "Ни один продукт из заказа не найден на складах. Доставка отменена.\n";
    }
}


void Truck::printStatistics() const {
    std::cout << "Статистика грузовика " << name << ":\n";
    std::cout << "Общий объем доставленного: " << total_delivered << " ед.\n";
    for (const auto& product : delivered_products) {
        std::cout << "Продукт: " << product.first << ", Доставлено: " << product.second << " ед.\n";
    }
}



int main() {
    std::cout<<"\n---ЗАГРУСКА СКЛАДОВ---\n\n";

    // Создаем склады с названиями и вместимостью
    Warehouse warehouseA("Склад A", 100);
    Warehouse warehouseB("Склад B", 100);
    std::vector<Warehouse*> warehouses = { &warehouseA, &warehouseB };

    // Создаем завод
    Factory factory1("Продукт A", 10.0, "Коробка", 90);
    Factory factory2("Продукт 1", 10.0, "Коробка", 90);
    Factory factory3("Продукт A", 10.0, "Коробка", 90);

    factory1.storage(warehouses);
    factory2.storage(warehouses);
    factory3.storage(warehouses);





    std::cout<<"\n---------СОЗДАНИЕ И ОБРАБОТКА ЗАПРОСА НА ДОСТАВКУ-----------\n\n";

    // Создаем грузовик
    Truck truck("Грузовик 1", 1000);
    Truck truck2("Грузовик 2", 1000);
    std::map<std::string, size_t> requests1 = {
            {"Продукт A", 1},
            {"Продукт 1", 1}
    };

    truck.deliver(warehouses, "Магазин 1", requests1);

    std::cout<<"\n-------ЖУРНАЛ ПОСТУПЛЕНИЙ-------\n\n";

    warehouseA.printArrivalLog();
    warehouseB.printArrivalLog();
    std::cout<<"\n---------СТАТИСТИКА ГРКЗОВИКОВ----------\n\n";

    truck.printStatistics();

    std::cout<<"\n---------ПРОВЕРКА АВТОВЫГРУЗКИ СКЛАДА--------\n\n";

    std::vector<Truck*> trucks = { &truck, &truck2 };
    warehouseA.autoUnload(trucks, "Магазин 1");

    return 0;
}
