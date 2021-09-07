# Score-P NVML Plugin
This code provides a Score-P plugin to access [NVIDIA Management Library (NVML)](https://developer.nvidia.com/nvidia-management-library-nvml)

## Installation
If not present on your system get NVML from [NVIDIA Management Library (NVML)](https://developer.nvidia.com/nvidia-management-library-nvml), install it with
`./gdk_linux_*_release.run --installdir=<PATH> --silent`
and setup paths

```
git clone --recurse-submodules git@github.com:score-p/scorep_plugin_nvml.git
cd scorep_plugin_nvml
mkdir build && cd build
cmake ../
make

# copy libnvml*_plugin.so into your LD_LIBRARY_PATH
```


## Usage
Sampling plugin seems to be more efficient than async (which samples via polling) or sync plugin but supports less
metrics. It uses special buffers on the GPU which continuesly sample.

> The advantage of using this method for samples in contrast to polling via existing methods is to get get higher frequency data at lower polling cost. 
> -- <cite>[NVML Docs](https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1gb7d2a6d2a9b4584cd985765d1ff46c94) </cite>

Be aware that some metrics are not supported on all devices, e.g. `utilization_gpu` in sampling mode can not be used on
NVIDIA K80 or GTX 1080 cards. The NVML documentation seems to be a bit vague.
### Sampling

- `SCOREP_METRIC_PLUGINS=nvml_sampling_plugin`
- `SCOREP_METRIC_NVML_SAMPLING_PLUGIN="utilization_gpu,power_usage"`
    
Optional :

- `SCOREP_METRIC_NVML_SAMPLING_PLUGIN_INTERVAL="5000"` (interval to poll the gpu in milliseconds, default 5000ms, note
  that this does not effect resolution of measurement, which is device and metric specific and can not be changed.
  Buffer sizes on the GPU also differ and can not be changed. Setting `SCOREP_METRIC_NVML_SAMPLING__PLUGIN_INTERVAL` to
  high will make you loose datapoints.)

#### Available metrics
- `clock_sm`
- `clock_mem`
- `power_usage`
- `utilization_gpu`
- `utilization_mem`

### Async

The a sync plugin polls devices every `SCOREP_METRIC_NVML_PLUGIN_INTERVAL` to get the current value.

- `SCOREP_METRIC_PLUGINS=nvml_plugin`
- `SCOREP_METRIC_NVML_PLUGIN="utilization_gpu,power_usage"`
    
Optional :
- `SCOREP_METRIC_NVML_PLUGIN_INTERVAL="50"` (measurement interval in milliseconds, default 50ms)
    
### Sync Plugin

The a sync plugin polls devices on trace events (e.g. `ENTER` and `LEAVE`) to get the current value.
- `SCOREP_METRIC_PLUGINS=nvml_plugin` 
- `SCOREP_METRIC_NVML_SYNC_PLUGIN="utilization_gpu,power_usage"`

#### Available metrics
- `clock_sm`
- `clock_mem`
- `fan_speed`
- `mem_free`
- `mem_used`
- `pcie_send`
- `pcie_recv`
- `power_usage`
- `temperature`
- `utilization_gpu`
- `utilization_mem`


## Developer note 
Current `nvml.h` can be found under 
https://github.com/NVIDIA/nvidia-settings/blob/master/src/nvml.h
