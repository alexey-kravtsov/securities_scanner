#include <config.h>

class OrdersLoader {
public:
    OrdersLoader();
    
    OrdersLoader(const OrdersLoader& other) = delete;
    OrdersLoader& operator=(const OrdersLoader& other) = delete;

    void load();
};