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

    std::map<std::string, Product> unload (size_t max_quantity) {
        std::map<std::string, Product> load;
        size_t total_units = 0;

        for (auto& item : inventory) {
            size_t quantity_to_take = std::min(item.second.quantity, max_quantity - total_units);
            if (quantity_to_take > 0) {
                load[item.first] = Product(item.first, item.second.weight, item.second.packaging, quantity_to_take);
                item.second.quantity -= quantity_to_take;
                current_load -= quantity_to_take;
                total_units += quantity_to_take;
                if (total_units >= max_quantity) break;
            }
        }

        std::cout << "Склад отгружен на " << total_units << " ед.\n";
        return load;
    }

    std::string getName() const {
        return name;
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

    void storage(std::vector<Warehouse>& warehouses) {
        Product product = createProduct();
        size_t remaining_quantity = product.quantity;

        // Сначала пытаемся найти склад, который может вместить весь продукт
        for (auto& warehouse : warehouses) {
            if (warehouse.getFreeSpace() >= remaining_quantity) {
                if (warehouse.storeProduct(product)) {
                    std::cout << "Продукт " << product.name << " полностью размещен на складе " << warehouse.getName() << "\n";
                    return; // Продукт успешно размещен
                }
            }
        }

        // Если не вмещается, распределяем по частям
        for (auto& warehouse : warehouses) {
            if (remaining_quantity == 0) {
                break;
            }

            size_t free_space = warehouse.getFreeSpace();
            if (free_space > 0) {
                size_t quantity_to_store = std::min(remaining_quantity, free_space);
                Product partial_product = Product(product.name, product.weight, product.packaging, quantity_to_store);
                if (warehouse.storeProduct(partial_product)) {
                    remaining_quantity -= quantity_to_store;
                }
            }
        }

        if (remaining_quantity > 0) {
            std::cout << "Не удалось сохранить всю продукцию " << product.name
                      << ": остаток " << remaining_quantity << " ед. на всех складах.\n";
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




int main() {
    // Создаем склады с названиями и вместимостью
    Warehouse warehouseA("Склад A", 100); // Склад A на 100 ед.
    Warehouse warehouseB("Склад B", 1); // Склад B на 100 ед.
    std::vector<Warehouse> warehouses = { warehouseA, warehouseB };

    // Создаем завод
    Factory factory("Продукт A", 10.0, "Коробка", 999); // производит 70 единиц в час

    // Запускаем процесс хранения продукции
    factory.storage(warehouses);

    return 0;
}
