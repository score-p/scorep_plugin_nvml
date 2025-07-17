#ifndef SCOREP_PLUGIN_NVML_NVML_WRAPPER_HPP
#define SCOREP_PLUGIN_NVML_NVML_WRAPPER_HPP

#include <nvml.h>

#include <stdexcept>
#include <string>
#include <vector>

enum metric_measure_type { ABS, REL, ACCU };

enum metric_datatype { INT, UINT, DOUBLE };

using pair_time_sampling_t = std::pair<unsigned long long, std::uint64_t>;

inline static void check_nvml_return(nvmlReturn_t ret,
                                     const std::string& str = "(not given)")
{
    if (NVML_SUCCESS != ret) {
        switch (ret) {
        case NVML_ERROR_UNINITIALIZED:
            throw std::runtime_error(
                "NVML was not first initialized with nvmlInit(): Error Code: " +
                std::to_string(ret));
        case NVML_ERROR_NOT_SUPPORTED:
            throw std::runtime_error(
                "NVML metric " + str +
                " is not supported on your device. Error Code: " + std::to_string(ret));
        case NVML_ERROR_NO_PERMISSION:
            throw std::runtime_error(
                "The current user does not have permission for operation " +
                str + ". Error Code: " + std::to_string(ret));
            ;
        default:
            throw std::runtime_error("An NVML error occured. Error Code: " +
                                     std::to_string(ret));
        }
    }
}

class Nvml_Metric {
public:
    virtual unsigned int get_value(nvmlDevice_t& device) = 0;

    const std::string& get_name() const
    {
        return name;
    }

    const std::string& get_desc() const
    {
        return desc;
    }

    const std::string& get_unit() const
    {
        return unit;
    }

    const metric_measure_type get_measure_type() const
    {
        return type;
    }

    const metric_datatype get_datatype() const
    {
        return datatype;
    }

protected:
    std::string name;
    std::string desc;
    std::string unit;
    metric_measure_type type;
    metric_datatype datatype;

protected:
    unsigned int value;
};

class Power : public Nvml_Metric {
public:
    Power(std::string name_ = "")
    {
        name = name_;
        desc = "Power Consumption";
        unit = "mW";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetPowerUsage(device, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Temperature : public Nvml_Metric {
public:
    Temperature(std::string name_ = "")
    {
        name = name_;
        desc = "Board Temperature";
        unit = "°C";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetTemperature(
            device, nvmlTemperatureSensors_t::NVML_TEMPERATURE_GPU, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Clock_Sm : public Nvml_Metric {
public:
    Clock_Sm(std::string name_ = "")
    {
        name = name_;
        desc = "SM clocks";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret =
            nvmlDeviceGetClockInfo(device, nvmlClockType_t::NVML_CLOCK_SM, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Clock_Mem : public Nvml_Metric {
public:
    Clock_Mem(std::string name_ = "")
    {
        name = name_;
        desc = "Memory clocks";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret =
            nvmlDeviceGetClockInfo(device, nvmlClockType_t::NVML_CLOCK_MEM, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Fan_Speed : public Nvml_Metric {
public:
    Fan_Speed(std::string name_ = "")
    {
        name = name_;
        desc = "Fan speed";
        unit = "";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetFanSpeed(device, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Mem_Free : public Nvml_Metric {
public:
    Mem_Free(std::string name_ = "")
    {
        name = name_;
        desc = "Free memory";
        unit = "Bytes";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }

    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlMemory_t mem;
        nvmlReturn_t ret = nvmlDeviceGetMemoryInfo(device, &mem);
        check_nvml_return(ret, name);
        value = mem.free;

        return value;
    }
};

class Mem_Used : public Nvml_Metric {
public:
    Mem_Used(std::string name_ = "")
    {
        name = name_;
        desc = "Used memory";
        unit = "Bytes";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlMemory_t mem;
        nvmlReturn_t ret = nvmlDeviceGetMemoryInfo(device, &mem);
        check_nvml_return(ret, name);
        value = mem.used;

        return value;
    }
};

class Mem_Total : public Nvml_Metric {
public:
    Mem_Total(std::string name_ = "")
    {
        name = name_;
        desc = "Total memory";
        unit = "Bytes";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlMemory_t mem;
        nvmlReturn_t ret = nvmlDeviceGetMemoryInfo(device, &mem);
        check_nvml_return(ret, name);
        value = mem.total;

        return value;
    }
};

class Module_Power_Instant : public Nvml_Metric {
public:
    Module_Power_Instant(std::string name_ = "")
    {
        name = name_;
        desc = "Module Instant Power";
        unit = "mW";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlFieldValue_t field_value;
        field_value.fieldId = NVML_FI_DEV_POWER_INSTANT;
        field_value.scopeId = 1;
        nvmlDeviceGetFieldValues(device, 1, &field_value);
        check_nvml_return(field_value.nvmlReturn, name);

        value = field_value.value.ulVal;

        return value;
    }
};

class Pcie_Send : public Nvml_Metric {
public:
    Pcie_Send(std::string name_ = "")
    {
        name = name_;
        desc = "PCIe Send";
        unit = "Bytes";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetPcieThroughput(
            device, nvmlPcieUtilCounter_t::NVML_PCIE_UTIL_TX_BYTES, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Pcie_Recv : public Nvml_Metric {
public:
    Pcie_Recv(std::string name_ = "")
    {
        name = name_;
        desc = "PCIe Recv";
        unit = "Bytes";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetPcieThroughput(
            device, nvmlPcieUtilCounter_t::NVML_PCIE_UTIL_RX_BYTES, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Utilization_Gpu : public Nvml_Metric {
public:
    Utilization_Gpu(std::string name_ = "")
    {
        name = name_;
        desc = "GPU Utilization";
        unit = "%";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlUtilization_t util;
        nvmlReturn_t ret = nvmlDeviceGetUtilizationRates(device, &util);
        check_nvml_return(ret, name);
        value = util.gpu;

        return value;
    }
};

class Utilization_Mem : public Nvml_Metric {
public:
    Utilization_Mem(std::string name_ = "")
    {
        name = name_;
        desc = "Memory Utilization";
        unit = "%";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlUtilization_t util;
        nvmlReturn_t ret = nvmlDeviceGetUtilizationRates(device, &util);
        check_nvml_return(ret, name);
        value = util.memory;

        return value;
    }
};

class Freq_Mem : public Nvml_Metric {
public:
    Freq_Mem(std::string name_ = "")
    {
        name = name_;
        desc = "Memory frequency";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetApplicationsClock(device, NVML_CLOCK_MEM, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Freq_Sm : public Nvml_Metric {
public:
    Freq_Sm(std::string name_ = "")
    {
        name = name_;
        desc = "SM frequency";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret = nvmlDeviceGetApplicationsClock(device, NVML_CLOCK_SM, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Freq_Graphics : public Nvml_Metric {
public:
    Freq_Graphics(std::string name_ = "")
    {
        name = name_;
        desc = "Graphics frequency";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;
    }
    unsigned int get_value(nvmlDevice_t& device)
    {
        nvmlReturn_t ret =
            nvmlDeviceGetApplicationsClock(device, NVML_CLOCK_GRAPHICS, &value);
        check_nvml_return(ret, name);

        return value;
    }
};

class Nvml_Sampling_Metric {
public:
    virtual ~Nvml_Sampling_Metric()
    {
        free(samples);
    }

    virtual std::vector<pair_time_sampling_t> get_value(nvmlDevice_t device,
                                                        unsigned long long last_seen = 0)
    {
        nvmlValueType_t val_type;
        unsigned int sample_count;

        std::vector<pair_time_sampling_t> vec;
        nvmlReturn_t ret;

        if (device == NULL) {
            throw std::runtime_error(
                "CUDA device for metric sampling not set.");
        }

        // get number of samples to allocate memory
        ret = nvmlDeviceGetSamples(device, sample_type, last_seen, &val_type,
                                   &sample_count, NULL);
        check_nvml_return(ret, name);
        vec.reserve(sample_count);

        if (sample_count > last_sample_count) {
            samples = (nvmlSample_t*)malloc(sample_count * sizeof(nvmlSample_t));
            if (samples == NULL) {
                throw std::runtime_error(
                    "Failed to allocate NVML sampling buffer");
            }
            last_sample_count = sample_count;
        }

        // get samples
        ret = nvmlDeviceGetSamples(device, sample_type, last_seen, &val_type,
                                   &sample_count, samples);

        if (NVML_SUCCESS != ret) {
            free(samples);
            throw std::runtime_error("Could not fetch data from NVML. " +
                                     std::string(nvmlErrorString(ret)) +
                                     "   Error Code: " + std::to_string(ret));
        }

        for (unsigned int i = 0; i < sample_count; ++i) {
            vec.emplace_back(samples[i].timeStamp, samples[i].sampleValue.uiVal);
        }

        return vec;
    }

    const std::string& get_name() const
    {
        return name;
    }

    const std::string& get_desc() const
    {
        return desc;
    }

    const std::string& get_unit() const
    {
        return unit;
    }

    const metric_measure_type get_measure_type() const
    {
        return type;
    }

    const metric_datatype get_datatype() const
    {
        return datatype;
    }

protected:
    std::string name;
    std::string desc;
    std::string unit;
    metric_measure_type type;
    metric_datatype datatype;

    nvmlSamplingType_t sample_type = nvmlSamplingType_t::NVML_GPU_UTILIZATION_SAMPLES;

private:
    nvmlSample_t* samples = NULL;
    unsigned int last_sample_count = 0;
};

class Power_Sampling : public Nvml_Sampling_Metric {
public:
    Power_Sampling(std::string name_ = "")
    {
        desc = "Power consumption (samples)";
        unit = "mW";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;

        sample_type = nvmlSamplingType_t::NVML_TOTAL_POWER_SAMPLES;
    }
};

class Utilization_Gpu_Sampling : public Nvml_Sampling_Metric {
public:
    Utilization_Gpu_Sampling(std::string name_ = "")
    {
        name = name_;
        desc = "GPU utilization (samples)";
        unit = "%";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;

        sample_type = nvmlSamplingType_t::NVML_GPU_UTILIZATION_SAMPLES;
    }
};

class Utilization_Mem_Sampling : public Nvml_Sampling_Metric {
public:
    Utilization_Mem_Sampling(std::string name_ = "")
    {
        name = name_;
        desc = "Memory utilization (samples)";
        unit = "%";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;

        sample_type = nvmlSamplingType_t::NVML_MEMORY_UTILIZATION_SAMPLES;
    }
};

class Clock_Sm_Sampling : public Nvml_Sampling_Metric {
public:
    Clock_Sm_Sampling(std::string name_ = "")
    {
        name = name_;
        desc = "SM clocks (sample)";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;

        sample_type = nvmlSamplingType_t::NVML_PROCESSOR_CLK_SAMPLES;
    }
};

class Clock_Mem_Sampling : public Nvml_Sampling_Metric {
public:
    Clock_Mem_Sampling(std::string name_ = "")
    {
        name = name_;
        desc = "Memory clocks (sample)";
        unit = "MHz";
        type = metric_measure_type::ABS;
        datatype = metric_datatype::UINT;

        sample_type = nvmlSamplingType_t::NVML_MEMORY_CLK_SAMPLES;
    }
};

Nvml_Metric* metric_name_2_nvml_function(std::string metric_name)
{
    Nvml_Metric* metric;
    if (metric_name.compare("power_usage") == 0) {
        metric = new Power(metric_name);
    }
    else if (metric_name.compare("temperature") == 0) {
        metric = new Temperature(metric_name);
    }
    else if (metric_name.compare("clock_sm") == 0) {
        metric = new Clock_Sm(metric_name);
    }
    else if (metric_name.compare("clock_mem") == 0) {
        metric = new Clock_Mem(metric_name);
    }
    else if (metric_name.compare("fan_speed") == 0) {
        metric = new Fan_Speed(metric_name);
    }
    else if (metric_name.compare("mem_free") == 0) {
        metric = new Mem_Free(metric_name);
    }
    else if (metric_name.compare("mem_used") == 0) {
        metric = new Mem_Used(metric_name);
    }
    else if (metric_name.compare("pcie_send") == 0) {
        metric = new Pcie_Send(metric_name);
    }
    else if (metric_name.compare("pcie_recv") == 0) {
        metric = new Pcie_Recv(metric_name);
    }
    else if (metric_name.compare("utilization_gpu") == 0) {
        metric = new Utilization_Gpu(metric_name);
    }
    else if (metric_name.compare("utilization_mem") == 0) {
        metric = new Utilization_Mem(metric_name);
    }
    else if (metric_name.compare("freq_sm") == 0) {
        metric = new Freq_Sm(metric_name);
    }
    else if (metric_name.compare("freq_mem") == 0) {
        metric = new Freq_Mem(metric_name);
    }
    else if (metric_name.compare("freq_graphics") == 0) {
        metric = new Freq_Graphics(metric_name);
    }
    else if (metric_name.compare("module_power_instant") == 0) {
        metric = new Module_Power_Instant(metric_name);
    }
    else {
        std::runtime_error("Unknown metric: " + metric_name);
    }
    return metric;
}

Nvml_Sampling_Metric* metric_name_2_nvml_sampling_function(std::string metric_name)
{
    Nvml_Sampling_Metric* metric;
    if (metric_name.compare("power_usage") == 0) {
        metric = new Power_Sampling(metric_name);
    }
    else if (metric_name.compare("clock_sm") == 0) {
        metric = new Clock_Sm_Sampling(metric_name);
    }
    else if (metric_name.compare("clock_mem") == 0) {
        metric = new Clock_Mem_Sampling(metric_name);
    }
    else if (metric_name.compare("utilization_gpu") == 0) {
        metric = new Utilization_Gpu_Sampling(metric_name);
    }
    else if (metric_name.compare("utilization_mem") == 0) {
        metric = new Utilization_Mem_Sampling(metric_name);
    }
    else {
        std::runtime_error("Unknown metric: " + metric_name);
    }
    return metric;
}

#endif // SCOREP_PLUGIN_NVML_NVML_WRAPPER_HPP
