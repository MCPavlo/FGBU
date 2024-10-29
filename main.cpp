#include <iostream>
#include <thread>
#include <map>
#include <algorithm>
#include <vector>
#include <string>

const size_t MIN_N = 50;
const size_t MIN_M = 100;
const size_t MIN_FACTORY_COUNT = 3;

struct Product {
    std::string name;
    double weight;
    std::string packaging;
    size_t quantity;

    Product(const std::string& name, double weight, const std::string& packaging, size_t quantity)
            : name(name), weight(weight), packaging(packaging), quantity(quantity) {}
    Product() : name(""), weight(0), packaging(""), quantity(0) {}
};

class Warehouse {
public:
    Warehouse(const std::string& name, size_t capacity)
            : name(name), capacity(capacity), current_load(0) {}

    size_t getFreeSpace() const {
        return capacity - current_load;
    }

    bool storeProduct(const Product& product) {
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

    std::map<std::string, Product> unload(const std::string& product_name, size_t max_quantity) {
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

        std::cout << "Склад отгружен на " << total_units << " ед. продукта " << product_name << ".\n";
        return load;
    }

    std::string getName() const {
        return name;
    }

    size_t getProductQuantity(const std::string& product_name) const {
        auto it = inventory.find(product_name);
        return (it != inventory.end()) ? it->second.quantity : 0;
    }

    void printArrivalLog() const {
        std::cout << "Журнал поступления продукции на склад " << name << ":\n";
        for (const auto& entry : arrival_log) {
            std::cout << "Фабрика: " << entry.factory_name << ", Продукт: " << entry.product_name
                      << ", Количество: " << entry.quantity << "\n";
        }
    }

private:
    struct ArrivalRecord {
        std::string factory_name;
        std::string product_name;
        size_t quantity;

        ArrivalRecord(const std::string& factory, const std::string& product, size_t qty)
                : factory_name(factory), product_name(product), quantity(qty) {}
    };

    std::string name;
    size_t capacity;
    size_t current_load;
    std::map<std::string, Product> inventory;
    std::vector<ArrivalRecord> arrival_log; // Вектор для хранения журналов поступления

    void recordArrival(const Product& product) {
        arrival_log.emplace_back("Фабрика", product.name, product.quantity); // Записываем поступление
    }
};

class Factory {
public:
    Factory(const std::string& name, double weight, const std::string& packaging, int production_rate)
            : name(name), weight(weight), packaging(packaging), production_rate(production_rate) {}

    void storage(std::vector<Warehouse*>& warehouses) {
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

private:
    std::string name;
    double weight;
    std::string packaging;
    int production_rate;

    Product createProduct() {
        return Product(name, weight, packaging, production_rate);
    }
};

class Truck {
public:
    Truck(const std::string& name, size_t max_capacity)
            : name(name), max_capacity(max_capacity), product_count(0), total_delivered(0) {}

    void loadProduct(const std::string& product_name, size_t count) {
        if (product_count + count > max_capacity) {
            std::cout << "Ошибка: не хватает места в грузовике " << name << " для загрузки "
                      << count << " ед. продукта " << product_name << ". Максимум: "
                      << max_capacity - product_count << " ед.\n";
            return; // Не загружаем, если превышаем максимум
        }

        product_count += count;
        total_delivered += count; // Увеличиваем общее количество доставленного
        delivered_products[product_name] += count; // Сохраняем информацию о доставленном продукте
        delivery_count[product_name]++; // Увеличиваем количество доставок для этого продукта
        std::cout << "Грузовик " << name << " загружен: " << count
                  << " ед. продукта " << product_name << ".\n";
    }

    void unloadProduct(const std::string& shop_name) {
        if (product_count > 0) {
            std::cout << "Грузовик " << name << " выгружает: "
                      << product_count << " ед. продукта в " << shop_name << ".\n";
            product_count = 0; // Сбрасываем количество после выгрузки
        } else {
            std::cout << "Грузовик " << name << " пуст.\n";
        }
    }

    void deliver(Warehouse* warehouse, const std::string& shop_name, const std::map<std::string, size_t>& requests) {
        size_t total_to_deliver = 0;

        for (const auto& request : requests) {
            const std::string& product_name = request.first;
            size_t quantity_needed = request.second;
            size_t available_quantity = warehouse->getProductQuantity(product_name);

            if (available_quantity < quantity_needed) {
                std::cout << "Недостаточно продукта " << product_name << " на складе " << warehouse->getName()
                          << ". Доступно: " << available_quantity << ", запрашиваемое: " << quantity_needed << ".\n";
            }

            total_to_deliver += std::min(quantity_needed, available_quantity);
        }

        if (total_to_deliver <= max_capacity) {
            for (const auto& request : requests) {
                const std::string& product_name = request.first;
                size_t quantity_needed = request.second;
                size_t loadable_count = std::min(quantity_needed, warehouse->getProductQuantity(product_name));

                if (loadable_count > 0) {
                    auto loaded_products = warehouse->unload(product_name, loadable_count);
                    for (const auto& item : loaded_products) {
                        loadProduct(item.first, item.second.quantity);
                    }
                }
            }
            unloadProduct(shop_name);
        } else {
            std::cout << "Превышен лимит загрузки для " << shop_name << ". Не удалось загрузить полный заказ.\n";
        }
    }

    // Новый перегруженный метод deliver, который принимает вектор складов
    void deliver(const std::vector<Warehouse*>& warehouses, const std::string& shop_name, const std::map<std::string, size_t>& requests) {
        size_t total_to_deliver = 0;

        // Подсчитаем общий объем для доставки по всем запросам
        for (const auto& request : requests) {
            const std::string& product_name = request.first;
            size_t quantity_needed = request.second;
            size_t available_quantity = 0;

            // Подсчитаем доступное количество по всем складам
            for (const auto& warehouse : warehouses) {
                available_quantity += warehouse->getProductQuantity(product_name);
                if (available_quantity >= quantity_needed) break;
            }

            total_to_deliver += std::min(quantity_needed, available_quantity);
        }

        // Если общий объем не превышает вместимость грузовика, загружаем все
        if (total_to_deliver <= max_capacity) {
            for (const auto& request : requests) {
                const std::string& product_name = request.first;
                size_t quantity_needed = request.second;
                size_t remaining_quantity = quantity_needed;

                for (auto& warehouse : warehouses) {
                    if (remaining_quantity == 0) break;

                    size_t available_quantity = warehouse->getProductQuantity(product_name);
                    size_t loadable_count = std::min(remaining_quantity, available_quantity);

                    if (loadable_count > 0) {
                        auto loaded_products = warehouse->unload(product_name, loadable_count);
                        for (const auto& item : loaded_products) {
                            loadProduct(item.first, item.second.quantity);
                            std::cout << "Загружено " << item.second.quantity << " ед. продукта "
                                      << item.first << " из склада " << warehouse->getName() << ".\n";
                        }
                        remaining_quantity -= loadable_count;
                    }
                }
            }
            unloadProduct(shop_name);
        } else {
            // Если общий объем превышает вместимость грузовика, загружаем по частям
            for (const auto& request : requests) {
                const std::string& product_name = request.first;
                size_t remaining_to_deliver = request.second;

                for (auto& warehouse : warehouses) {
                    while (remaining_to_deliver > 0) {
                        size_t available_quantity = warehouse->getProductQuantity(product_name);
                        if (available_quantity == 0) break;

                        size_t loadable_count = std::min(remaining_to_deliver, std::min(available_quantity, max_capacity - product_count));

                        if (loadable_count > 0) {
                            auto loaded_products = warehouse->unload(product_name, loadable_count);
                            for (const auto& item : loaded_products) {
                                loadProduct(item.first, item.second.quantity);
                                std::cout << "Загружено " << item.second.quantity << " ед. продукта "
                                          << item.first << " из склада " << warehouse->getName() << ".\n";
                            }
                            remaining_to_deliver -= loadable_count;
                        }

                        if (product_count >= max_capacity) {
                            unloadProduct(shop_name);
                        }
                    }

                    if (remaining_to_deliver == 0) break;
                }

                if (product_count > 0) {
                    unloadProduct(shop_name);
                }
            }
        }
    }


    void printStatistics() const {
        std::cout << "Грузовик " << name << " перевез " << total_delivered << " ед. продукции.\n";
        std::cout << "Статистика по доставленной продукции:\n";

        for (const auto& entry : delivered_products) {
            std::cout << "Продукт: " << entry.first << ", Количество: " << entry.second << "\n";
            if (delivery_count.at(entry.first) > 0) {
                double average_delivery = static_cast<double>(entry.second) / delivery_count.at(entry.first);
                std::cout << "Среднее количество доставленного продукта " << entry.first << ": " << average_delivery << "\n";
            }
        }

        std::cout << "Текущая загрузка грузовика: " << product_count << " ед.\n";
        std::cout << "Максимальная грузоподъемность грузовика: " << max_capacity << " ед.\n";

        // Выводим среднее количество каждого продукта, который возит грузовик
        for (const auto& entry : delivered_products) {
            if (delivery_count.at(entry.first) > 0) {
                double average_per_product = static_cast<double>(entry.second) / delivery_count.at(entry.first);
                std::cout << "В среднем грузовик " << name << " возит " << average_per_product
                          << " ед. продукта \"" << entry.first << "\".\n";
            }
        }
    }

private:
    std::string name;
    size_t max_capacity; // Максимальная грузоподъемность грузовика
    size_t product_count; // Текущая загрузка грузовика
    size_t total_delivered; // Общее количество доставленной продукции
    std::map<std::string, size_t> delivered_products; // Статистика по доставленной продукции
    std::map<std::string, size_t> delivery_count; // Количество доставок для каждого продукта
};



int main() {
    std::cout<<"\n"<<"---ЗАГРУСКА СКЛАДОВ---\n"<<"\n";

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

    std::cout<<"\n"<<"---------СОЗДАНИЕ И ОБРАБОТКА ЗАПРОСА НА ДОСТАВКУ-----------\n"<<"\n";

    // Создаем грузовик
    Truck truck("Грузовик 1", 1000);

    std::map<std::string, size_t> requests1 = {
            {"Продукт A", 15},
            {"Продукт 1", 10}
    };

    truck.deliver(warehouses, "Магазин 1", requests1);

    std::cout<<"\n-------ЖУРНАЛ ПОСТУПЛЕНИЙ-------\n\n";

    warehouseA.printArrivalLog();
    warehouseB.printArrivalLog();
    std::cout<<"\n---------СТАТИСТИКА ГРКЗОВИКОВ----------\n\n";

    truck.printStatistics();

    return 0;
}
