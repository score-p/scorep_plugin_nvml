#include "nvml_scorep_helper.hpp"
#include "nvml_types.hpp"
#include "nvml_wrapper.hpp"

#include <scorep/plugin/plugin.hpp>

#include <nvml.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace scorep::plugin::policy;

using scorep::plugin::logging;

template <typename T, typename Policies>
using nvml_object_id =
    scorep::plugin::policy::object_id<nvml_t<Nvml_Metric>, T, Policies>;

class nvml_sync_plugin
    : public scorep::plugin::base<nvml_sync_plugin, sync, per_host, scorep_clock, nvml_object_id> {
public:
    nvml_sync_plugin()
    {
        nvmlReturn_t nvml = nvmlInit_v2();
        if (NVML_SUCCESS != nvml) {
            throw std::runtime_error("Could not start NVML. Code: " +
                                     std::string(nvmlErrorString(nvml)));
        }
    }

    ~nvml_sync_plugin()
    {
        nvmlReturn_t nvml = nvmlShutdown();
        if (NVML_SUCCESS != nvml) {
            //            throw std::runtime_error("Could not terminate NVML. Code: " +
            //                                     std::string(nvmlErrorString(nvml)));
            logging::warn() << "Could not terminate NVML. Code:"
                            << std::string(nvmlErrorString(nvml));
        }
    }

    // Convert a named metric (may contain wildcards or so) to a vector of
    // actual metrics (may have a different name)
    std::vector<scorep::plugin::metric_property> get_metric_properties(const std::string& metric_name)
    {
        std::vector<scorep::plugin::metric_property> properties;

        logging::info()
            << "nvml_sync_plugin::get_metric_properties() called with: " << metric_name;

        Nvml_Metric* metric_type = metric_name_2_nvml_function(metric_name);

        std::vector<nvmlDevice_t> nvml_devices;
        for (unsigned int i = 0; i < nvml_devices.size(); ++i) {
            /* TODO use device index by nvmlDeviceGetIndex( nvmlDevice_t device, unsigned int* index ) */

            std::string new_name = metric_name + " on CUDA: " + std::to_string(i);
            auto handle = make_handle(
                new_name, nvml_t<Nvml_Metric>{metric_name, nvml_devices[i], metric_type});

            scorep::plugin::metric_property property = scorep::plugin::metric_property(
                new_name, metric_type->get_desc(), metric_type->get_unit());

            if (!set_scorep_datatype(metric_type, property)) {
                throw std::runtime_error("Unknown datatype for metric " + metric_name);
            }

            if (!set_scorep_measure_type(metric_type, property)) {
                throw std::runtime_error("Unknown measure type for metric " + metric_name);
            }

            properties.push_back(property);
        }
        return properties;
    }

    void add_metric(nvml_t<Nvml_Metric>& handle)
    {
        logging::info() << "add metric called with: " << handle.name
                        << " on CUDA " << handle.device_idx;
    }

    template <typename P>
    bool get_optional_value(nvml_t<Nvml_Metric>& handle, P& proxy)
    {
        logging::info() << "get_optional_value called with: " << handle.name
                        << " CUDA " << handle.device_idx;

        std::uint64_t data = handle.metric->get_value(handle.device);
        proxy.write(data);
        return true;
    }

private:
    std::vector<nvmlDevice_t> get_visible_devices()
    {
        std::vector<nvmlDevice_t> devices;

        nvmlReturn_t ret;
        unsigned int num_devices;

        ret = nvmlDeviceGetCount(&num_devices);
        if (NVML_SUCCESS != ret) {
            throw std::runtime_error(nvmlErrorString(ret));
        }

        /*
         * New nvmlDeviceGetCount_v2 (default in NVML 5.319) returns count of all devices in the system
         * even if nvmlDeviceGetHandleByIndex_v2 returns NVML_ERROR_NO_PERMISSION for such device.
         */
        nvmlDevice_t device;
        for (unsigned i = 0; i < num_devices; ++i) {
            ret = nvmlDeviceGetHandleByIndex(i, &device);

            if (NVML_SUCCESS == ret) {
                devices.push_back(device);
            }
            else if (NVML_ERROR_NO_PERMISSION == ret) {
                logging::info() << "No permission for device: " << i;
            }
            else {
                throw std::runtime_error(nvmlErrorString(ret));
            }
        }
        return devices;
    }
};