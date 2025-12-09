# PlaneControl HAL Test Suite

This repository contains the Vendor Test Suite (VTS) for the `IPlaneControl` AIDL HAL interface. It is designed to validate the functionality of the PlaneControl HAL implementation.

## Build Steps

This test suite is part of Unified VTS for AIDL HALs. For initial setup, environment configuration, and build instructions, please refer to the Unified VTS README documentation.

- **[Unified VTS README](https://github.com/rdkcentral/rdk-halif-aidl-unified-vts/blob/main/README.md)**

## Running the PlaneControl Tests

While the Unified VTS can run all tests at once, you can execute the `VTS_L1_PlaneControl`, `VTS_L2_PlaneControl` and `VTS_L3_PlaneControl` test suite individually for targeted validation. This is useful for debugging and development of the `PlaneControl` HAL.

## Reference Documents

|SNo|Document Name|Document Description|Document Link|
|---|-------------|--------------------|-------------|
|1|High Level Test Specification Document|High Level Test Specification Documentation for this module|[planecontrol_High-Level_TestSpecification.md](docs/pages/planecontrol_High-Level_TestSpecification.md)|
|2|`L2` Low Level Test Specification Document|Low Level Test Specification Documentation for this module|[planecontrol_L2_Low-Level_TestSpecification.md](docs/pages/planecontrol_L2_Low-Level_TestSpecification.md)|
