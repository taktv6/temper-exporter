#include <gsl/gsl>
#include <iostream>
#include <condition_variable>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include "temper.hpp"

class TempCollector: public virtual prometheus::Collectable {
    Temper::Temper * temper;

    public:
        TempCollector(Temper::Temper * temper);
        std::vector<prometheus::MetricFamily> Collect() const override;
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

    return EXIT_SUCCESS;
}

