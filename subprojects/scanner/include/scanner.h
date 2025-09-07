#include <config.h>
#include <memory>
#include <bonds_loader.h>
#include <orders_loader.h>

class Scanner {
    public:
        Scanner(const Config& config);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void init();
        void start();
    
    private:
        std::unique_ptr<BondsLoader> bonds_loader;
        std::unique_ptr<OrdersLoader> orders_loader;
};