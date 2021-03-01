#ifndef SCOREP_PLUGIN_NVML_NVML_TYPES_HPP
#define SCOREP_PLUGIN_NVML_NVML_TYPES_HPP

#include "nvml_wrapper.hpp"

#include <nvml.h>

#include <scorep/chrono/chrono.hpp>
#include <scorep/plugin/plugin.hpp>

using system_clock_t = std::chrono::system_clock;
using system_time_point_t = std::chrono::time_point<system_clock_t>;

using pair_chrono_value_t = std::pair<system_time_point_t, std::uint64_t>;

using scorep::plugin::logging;

template <typename T>
class nvml_t {
public:
    nvml_t(const std::string& name_, nvmlDevice_t device_, T* metric_)
        : name(name_), device(device_), metric(metric_)
    {
        nvmlReturn_t ret = nvmlDeviceGetIndex(device, &device_idx);
        check_nvml_return(ret);
    }
    ~nvml_t()
    {
        logging::info() << "call destructor of " << name;
    }

    //    // delete copy constructor to avoid ... copies,
    //    // needs move and default constructor though!
    //    nvml_t(nvml_t&&) = default;
    //    nvml_t(const nvml_t&) = delete;
    //    /* copy-assign */
    //    nvml_t& operator=(const nvml_t&) = delete;
    //    /* move assignment */
    //    nvml_t& operator=(nvml_t&&) = default;

    bool operator==(const nvml_t& other) const
    {
        return (this->name == other.name) && (this->device_idx == this->device_idx);
    }

    std::string name;
    T* metric;
    unsigned int device_idx;
    nvmlDevice_t device;
};

namespace std {
/** operator to print the metric handle
 */
template <typename T>
inline ostream& operator<<(ostream& s, const nvml_t<T>& metric)
{
    s << "(" << metric.name << " on CUDA " << std::to_string(metric.device_idx) << ")";
    return s;
}

/** hashing using the metric name and device id
 */
template <typename T>
struct hash<nvml_t<T>> {
    size_t inline operator()(const nvml_t<T>& metric) const
    {
        return std::hash<std::string>{}(metric.name + std::to_string(metric.device_idx));
    }
};
};     // namespace std
#endif // SCOREP_PLUGIN_NVML_NVML_TYPES_HPP