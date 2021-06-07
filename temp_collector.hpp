#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include "temper.hpp"

class TempCollector: public virtual prometheus::Collectable {
    Temper::Temper *temper;

    public:
        TempCollector(Temper::Temper *temper);
        std::vector<prometheus::MetricFamily> Collect() const override;
};