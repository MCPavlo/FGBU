#ifndef CLASSES_H
#define CLASSES_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class Product {
public:
    Product(const std::string& name, double weight, const std::string& packaging, size_t quantity);
    Product();

    std::string name;
    double weight;
    std::string packaging;
    size_t quantity;
    [[nodiscard]] size_t getQuantity() const { return quantity; }
    void decreaseQuantity(size_t amount) { quantity -= amount; }
    [[nodiscard]] std::string getName() const{return name;}
};

class Warehouse {
public:
    void startAutoUnload(std::vector<class Truck*>& trucks, const std::string& shop_name);
    Warehouse(const std::string& name, size_t capacity);
    size_t getFreeSpace() const;
    bool storeProduct(const Product& product);
    std::map<std::string, Product> unload(const std::string& product_name, size_t max_quantity);
    std::string getName() const;
    size_t getProductQuantity(const std::string& product_name) const;
    void printArrivalLog() const;
    bool isOverloaded() const;
    void autoUnload(std::vector<class Truck*>& trucks, const std::string& shop_name);

private:
    mutable std::mutex mtx;
    struct ArrivalLogEntry {
        std::string factory_name;
        std::string product_name;
        size_t quantity;
        ArrivalLogEntry(const std::string& factory_name, const std::string& product_name, size_t quantity)
                : factory_name(factory_name), product_name(product_name), quantity(quantity) {}
    };

    std::string name;
    size_t capacity;
    size_t current_load;
    std::map<std::string, Product> inventory;
    std::vector<ArrivalLogEntry> arrival_log;
    std::atomic<bool> is_unloading{false};

    void recordArrival(const Product& product);
};

class Factory {
public:
    Factory(const std::string& name, double weight, const std::string& packaging, int production_rate);
    void storage(std::vector<Warehouse*>& warehouses);
    Product createProduct();

private:
    std::string name;
    double weight;
    std::string packaging;
    int production_rate;
};

class Truck {
public:
    Truck(const std::string& name, size_t max_capacity);
    void loadProduct(const std::string& product_name, size_t count);
    void unloadProduct(const std::string& shop_name);
    void deliver(Warehouse* warehouse, const std::string& shop_name, const std::map<std::string, size_t>& requests);
    void deliver(const std::vector<Warehouse*>& warehouses, const std::string& shop_name, const std::map<std::string, size_t>& requests);
    void printStatistics() const;
    size_t getCapacity() {return max_capacity;}
    size_t getCurrentLoad() const { return product_count; } // Add this method
    std::string getName(){return name;}

    void addProduct(const std::string& product_name, size_t count) {
        if (product_count + count <= max_capacity) {
            product_count += count;
            total_delivered += count; // Увеличиваем общее количество доставленного
            delivered_products[product_name] += count; // Увеличиваем количество доставленного конкретного продукта

            std::cout << "Загружено " << count << " ед. продукта " << product_name << " в грузовик " << name << ".\n";
        } else {
            std::cout << "Ошибка: не хватает места в грузовике " << name << " для загрузки " << count << " ед. продукта " << product_name << ".\n";
        }
    }

    mutable std::mutex mtx;
private:
    std::string name;
    size_t max_capacity;
    size_t product_count;
    size_t total_delivered;
    std::map<std::string, size_t> loadedProducts;
    std::map<std::string, size_t> delivered_products;
    std::map<std::string, size_t> delivery_count;
};

#endif // CLASSES_H
