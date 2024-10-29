#include <iostream>
#include <thread>
#include <map>
#include <algorithm>
#include <vector>

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

private:
    std::string name;
    size_t capacity;
    size_t current_load;
    std::map<std::string, Product> inventory;
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
    Truck(const std::string& name)
            : name(name), product_name(""), product_count(0) {}

    void loadProduct(const std::string& product_name, size_t count) {
        this->product_name = product_name;
        this->product_count += count;
        std::cout << "Грузовик " << name << " загружен: " << count
                  << " ед. продукта " << product_name << ".\n";
    }

    void unloadProduct(const std::string& shop_name) {
        if (product_count > 0) {
            std::cout << "Грузовик " << name << " выгружает: "
                      << product_count << " ед. продукта " << product_name
                      << " в " << shop_name << ".\n";
            product_count = 0;
            product_name = "";
        } else {
            std::cout << "Грузовик " << name << " пуст.\n";
        }
    }

    void deliver(Warehouse* warehouse, const std::string& shop_name, const std::string& product_name, size_t count) {
        // Проверяем, есть ли достаточно продукта на складе
        size_t available_quantity = warehouse->getProductQuantity(product_name);
        if (available_quantity < count) {
            std::cout << "Недостаточно продукта " << product_name << " на складе " << warehouse->getName()
                      << ". Доступно: " << available_quantity << ", запрашиваемое: " << count << ".\n";
            return;
        }

        // Загружаем продукцию из склада
        auto loaded_products = warehouse->unload(product_name, count);
        for (const auto& item : loaded_products) {
            loadProduct(item.first, item.second.quantity);
        }

        // Выгружаем продукцию в магазин
        unloadProduct(shop_name);
    }

private:
    std::string name;
    std::string product_name;
    size_t product_count;
};

int main() {
    // Создаем склады с названиями и вместимостью
    Warehouse warehouseA("Склад A", 100);
    Warehouse warehouseB("Склад B", 1);
    std::vector<Warehouse*> warehouses = { &warehouseA, &warehouseB }; // Используем указатели

    // Создаем завод
    Factory factory("Продукт A", 10.0, "Коробка", 90); // производит 90 единиц

    // Запускаем процесс хранения продукции
    factory.storage(warehouses);

    // Создаем грузовик
    Truck truck("Грузовик 1");

    // Грузим продукцию из склада в магазин
    truck.deliver(&warehouseA, "Магазин 1", "Продукт A", 50); // Должно сработать

    // Попробуем еще раз забрать 40 единиц, теперь доступно только 40
    truck.deliver(&warehouseA, "Магазин 1", "Продукт A", 40); // Это вызовет сообщение о недостатке

    return 0;
}
