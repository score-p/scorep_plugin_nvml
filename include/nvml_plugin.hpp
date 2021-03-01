#include "nvml.h"
#include "nvml_measurement_thread.hpp"
#include "nvml_scorep_helper.hpp"
#include "nvml_types.hpp"
#include "nvml_wrapper.hpp"

#include <scorep/plugin/plugin.hpp>

#include <nvml.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace scorep::plugin::policy;

using scorep::plugin::logging;

template <typename T, typename Policies>
using nvml_object_id =
    scorep::plugin::policy::object_id<nvml_t<Nvml_Metric>, T, Policies>;

class nvml_plugin
    : public scorep::plugin::base<nvml_plugin, async, per_host, scorep_clock, post_mortem, nvml_object_id> {
public:
    nvml_plugin()
        : nvml_m(std::chrono::milliseconds(
              stoi(scorep::environment_variable::get("interval", "50"))))
    {
        nvmlReturn_t ret = nvmlInit_v2();
        check_nvml_return(ret);
    }

    ~nvml_plugin()
    {
        nvmlReturn_t ret = nvmlShutdown();
        check_nvml_return(ret);
    }

    // Convert a named metric (may contain wildcards or so) to a vector of
    // actual metrics (may have a different name)
    std::vector<scorep::plugin::metric_property> get_metric_properties(const std::string& metric_name)
    {
        std::vector<scorep::plugin::metric_property> properties;

        logging::info() << "nvml_plugin::get_metric_properties() called with: " << metric_name;

        Nvml_Metric* metric_type = metric_name_2_nvml_function(metric_name);

        std::vector<nvmlDevice_t> nvml_devices = get_visible_devices();
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

        nvml_m.add_handles(get_handles());

        return properties;
    }

    void add_metric(nvml_t<Nvml_Metric>& handle)
    {
        logging::info() << "add metric called with: " << handle.name
                        << " on CUDA " << handle.device_idx;
    }

    // start your measurement in this method
    void start()
    {
        nvml_thread = std::thread([this]() { this->nvml_m.measurement(); });

        time_converter.synchronize_point(
            nvml_m.get_timepoint(), scorep::chrono::measurement_clock::now());

        logging::info() << "Successfully started NVML measurement.";
    }

    // stop your measurement in this method
    void stop()
    {
        time_converter.synchronize_point(
            nvml_m.get_timepoint(), scorep::chrono::measurement_clock::now());

        nvml_m.stop_measurement();
        if (nvml_thread.joinable()) {
            nvml_thread.join();
        }

        logging::info() << "Successfully stopped NVML measurement.";
    }

    // Will be called post mortem by the measurement environment
    // You return all values measured.
    template <typename C>
    void get_all_values(nvml_t<Nvml_Metric>& handle, C& cursor)
    {
        logging::info() << "get_all_values called with: " << handle.name
                        << " CUDA " << handle.device_idx;

        auto values = nvml_m.get_readings(handle);
        for (auto& value : values) {
            cursor.write(time_converter.to_ticks(value.first), value.second);
        }

        logging::debug() << "get_all_values wrote " << values.size() << " values (out of which "
                         << cursor.size() << " are in the valid time range)";
    }

private:
    scorep::chrono::time_convert<> time_converter;

    nvml_measurement_thread<Nvml_Metric> nvml_m;
    std::thread nvml_thread;

private:
    std::vector<nvmlDevice_t> get_visible_devices()
    {
        std::vector<nvmlDevice_t> devices;

        nvmlReturn_t ret;
        unsigned int num_devices;

        ret = nvmlDeviceGetCount(&num_devices);
        check_nvml_return(ret);

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
    //    void nvml_check_return(const nvmlReturn_t& ret)
    //    {
    //        if (NVML_SUCCESS != ret) {
    //            throw std::runtime_error(std::string(("NVML error: ")) + nvmlErrorString(ret));
    //        }
    //    }
};