#include <gsl/gsl>
#include <iostream>
#include <condition_variable>
#include <unistd.h>

#include <boost/exception/diagnostic_information.hpp> 

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include "temper.hpp"

class TempCollector: public virtual prometheus::Collectable {
    public:
        TempCollector(Temper::Temper * temper);
        std::vector<prometheus::MetricFamily> Collect() const override;

    private:
        Temper::Temper * temper;
};

TempCollector::TempCollector(Temper::Temper * temper) {
    this->temper = temper;
}

std::vector<prometheus::MetricFamily> TempCollector::Collect() const {
    return std::vector<prometheus::MetricFamily>{
        {
            name: "temper_temperature_celsius",
            help: "TEMPer1F temperature in celsius",
            type: prometheus::MetricType::Gauge,
            metric: std::vector<prometheus::ClientMetric>{
                {
                    label: std::vector<prometheus::ClientMetric::Label>{},
                    gauge: prometheus::ClientMetric::Gauge{(double)(this->temper->getTemp())},
                }
            }
        }
    };
}

void block() {
    std::condition_variable exit_cond;
    std::mutex mut;
    std::unique_lock<std::mutex> lk(mut);
    exit_cond.wait(lk);
}

int main() {
    auto registry = std::make_shared<prometheus::Registry>();

    try {
        prometheus::Exposer exposer{"0.0.0.0:8080, [::]:8080"};
        exposer.RegisterCollectable(registry);

        Temper::Temper * temper;

        temper = new Temper::Temper();

        auto ___ = gsl::finally([&temper]{
            delete temper;
        });

        auto temp_col = std::make_shared<TempCollector>(temper);
        exposer.RegisterCollectable(temp_col);

        block();
    }
    catch (const std::runtime_error e) {
        std::clog << "An exception occured: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

