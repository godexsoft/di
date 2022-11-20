#include <di.hpp>
using namespace di;

class LogService {
public:
    ~LogService() { std::cout << "~LogService\n"; }
    void debug(std::string str) const { std::cout << str << '\n'; }
    void mutating() {}
};

class NetworkService {
public:
    using services_t = Services<LogService>;
    NetworkService(services_t services)
        : services_{ services } {}
    ~NetworkService() { std::cout << "~NetworkService\n"; }

    void send(std::string str) const {
        services_.get<LogService>()->debug("NetworkService: " + str);
    }

private:
    services_t services_;
};

class Watchdog {
public:
    using services_t = Services<const LogService, NetworkService>;
    using logger_t   = std::shared_ptr<const LogService>;

    Watchdog(services_t services)
        : services_{ services } {}
    ~Watchdog() { std::cout << "~Watchdog\n"; }

    void test() const {
        logger_->debug("Running watchdog...");
        // can't call: services_.get<LogService>()->mutating();
        services_.get<NetworkService>()->send("Watching bruh");
        logger_->debug("-- watchdog... --");
    }

private:
    services_t services_;
    logger_t logger_ = services_.get<LogService>();
};

int main() {
    auto log_service = std::make_shared<LogService>();
    auto net_service = std::make_shared<NetworkService>(log_service);
    auto watchdog    = std::make_shared<Watchdog>(
        Services(log_service, net_service)); // log_service is const inside watchdog

    watchdog->test();
    return 0;
}
