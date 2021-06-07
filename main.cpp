#include <gsl/gsl>
#include <iostream>
#include <condition_variable>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include "temp_collector.hpp"
#include "temper.hpp"

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
