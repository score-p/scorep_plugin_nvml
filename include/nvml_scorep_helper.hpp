#ifndef SCOREP_PLUGIN_NVML_NVML_SCOREP_HELPER_HPP
#define SCOREP_PLUGIN_NVML_NVML_SCOREP_HELPER_HPP

#include <scorep/plugin/plugin.hpp>

#include "nvml_wrapper.hpp"

template <typename T>
static bool set_scorep_measure_type(const T* metric_type,
                                    scorep::plugin::metric_property& property)
{
    metric_measure_type measure_type = metric_type->get_measure_type();
    switch (measure_type) {
    case ABS:
        property.absolute_point();
        break;
    case REL:
        property.relative_point();
        break;
    case ACCU:
        property.accumulated_point();
        break;
    default:
        return false;
    }
    return true;
}

template <typename T>
static bool set_scorep_datatype(const T* metric_type, scorep::plugin::metric_property& property)
{
    metric_datatype datatype = metric_type->get_datatype();
    switch (datatype) {
    case UINT:
        property.value_uint();
        break;
    case INT:
        property.value_int();
        break;
    case DOUBLE:
        property.value_double();
        break;
    default:
        return false;
    }
    return true;
}

#endif // SCOREP_PLUGIN_NVML_NVML_SCOREP_HELPER_HPP
