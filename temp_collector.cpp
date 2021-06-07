#include "temp_collector.hpp"

TempCollector::TempCollector(Temper::Temper *temper) {
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