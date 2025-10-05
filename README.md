## YODPS — Yet One Distributed Processing System
© 2025 Yury Odnokurtsev / Urry Kurtz

**YODPS** is a high-performance, modular framework for distributed data acquisition, processing, visualization, and recording — designed as a flexible alternative to commercial automotive/robotic middleware solutions, where such systems exist.

## ✨ Features

**Communication layer** based on [ZeroMQ](https://github.com/zeromq/libzmq), supporting **TCP**, **IPC**, and **shared memory** transport, with or without a broker.

**Cross-platform**: x64 Linux & Windows, ARM64 Linux & QNX, Android, WebAssembly.

**Extensible viewer application** using SDL+Urho3D and ImGui with next features:
  + Multi-camera video streaming
  + Creating and rendering 3D geometric objects and complex models in popular formats (OBJ, STL etc)
  + GUI overlay
  + Data viewer / plotter / generator
  + CAN/CANFD signal viewer
  + OBD2 param viewer
  + Simple Plugin API to extend functionality
  
**Default data acquisition nodes**:
  - PCAP/libtins (CAN/CANFD, UDP/TCP)
  - V4L2 cameras
  - Image/Video format converters
  - Audio/MIDI via ALSA/JACK Audio Connection Kit
  
**Recorder/Player**: 
  - ADTF DAT v2.0 file format (using `adtf_file` library) 
  - Foxglove studio MCAP
  - PCAP

## 🚗 Deployment

YODPS target platforms include on-road and off-road vehicles, drones, marine vessels, locomotives, industrial machinery, specialized defense systems, including giant humanoid robots.

## Screenshots 
![](https://github.com/UrryKurtz/YODPS/blob/main/Doc/Screenshot_002.png?raw=true)
![](https://github.com/UrryKurtz/YODPS/blob/main/Doc/Screenshot_003.png?raw=true)

## Videos
**Camera control, Video and Polyline Plugins demo**

[![Watch the video](https://img.youtube.com/vi/yHRvpe6Kq-4/hqdefault.jpg)](https://youtu.be/yHRvpe6Kq-4 "Watch on YouTube")

**Open Street Map GPS Plugin**

[![Watch the video](https://img.youtube.com/vi/4tj1m2y9mRg/hqdefault.jpg)](https://youtu.be/4tj1m2y9mRg "Watch on YouTube")

**Plotter Plugin**

[![Watch the video](https://img.youtube.com/vi/LsX0EGzu1fc/hqdefault.jpg)](https://youtu.be/LsX0EGzu1fc "Watch on YouTube")


## 📦 Third-party dependencies

- [ZeroMQ (libzmq)](https://github.com/zeromq/libzmq) — MPL 2.0  
- [MCAP](https://github.com/foxglove/mcap) — MIT License  
- [libtins](https://github.com/mfontanini/libtins) — BSD 2-Clause License  
- [LVGL](https://github.com/lvgl/lvgl) — MIT License  
- [msgpack-c](https://github.com/msgpack/msgpack-c) — Boost Software License 1.0  
- [Urho3D](https://github.com/urho3d/Urho3D) — MIT License
- [rbfx](https://github.com/rbfx/rbfx) — MIT License
- [Dear ImGui](https://github.com/ocornut/imgui) — MIT License
- [httplib](https://github.com/yhirose/cpp-httplib) — MIT License 

## 📜 License

This software is dual-licensed:

1. **Open Source License (GPLv3)**  
   You may use, modify, and distribute this software under the terms of the GNU General Public License version 3 (GPLv3).  
   See the [LICENSE.md](LICENSE.md) file for full details.

2. **Commercial License**  
   If you wish to use this software in a closed-source product, or without the obligations of GPLv3,  
   a commercial license is available for purchase.  
   Contact: **y.odnokurtsev@gmail.com**

By using this software, you agree to comply with the terms of one of the above licenses.

---

