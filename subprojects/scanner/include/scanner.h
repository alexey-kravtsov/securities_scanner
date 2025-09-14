#include <config.h>
#include <memory>
#include <bonds_loader.h>
#include <orders_loader.h>

class Scanner {
    public:
        Scanner(const Config& config, BondsLoader& bonds_loader, OrdersLoader& orders_loader);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void init();
        void start();
    
    private:
        const Config& config;
        BondsLoader& bonds_loader;
        OrdersLoader& orders_loader;
};