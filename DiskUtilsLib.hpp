#ifndef DISKUTILSLIB_H
#define DISKUTILSLIB_H

#include <vector>
#include <string>
#include <windows.h>
#include <iostream>

/*
    DiskUtilsLib - A C++ library for retrieving disk properties.

    Author: dwep
    GitHub: https://github.com/dwep1337
    License: MIT License (see LICENSE file)

    Description:
    This library provides functionality to fetch disk properties including
    serial number, model, and size for Windows systems. It utilizes Windows
    API functions to gather detailed information about the disk drives.

    Features:
    - Retrieve Disk Serial Number
    - Retrieve Disk Model
    - Retrieve Disk Size

    Requirements:
    - Windows OS
    - C++11 or later compiler
*/

class DiskUtils {
public:
    /**
     * Retrieves the disk numbers for the specified volume.
     * @param volumePath The path to the volume (e.g., L"\\\\.\\C:").
     * @param diskNumbers A vector to store the disk numbers associated with the volume.
     * @return true if the operation was successful, otherwise false.
     */
    static bool GetVolumeDiskExtents(const std::wstring& volumePath, std::vector<int>& diskNumbers) {
        HANDLE hVolume = CreateFileW(volumePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hVolume == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open volume. Error: " << GetLastError() << std::endl;
            return false;
        }

        VOLUME_DISK_EXTENTS diskExtents;
        DWORD bytesReturned;
        if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &diskExtents, sizeof(diskExtents), &bytesReturned, NULL)) {
            std::cerr << "Failed to get volume disk extents. Error: " << GetLastError() << std::endl;
            CloseHandle(hVolume);
            return false;
        }

        for (DWORD i = 0; i < diskExtents.NumberOfDiskExtents; ++i) {
            diskNumbers.push_back(diskExtents.Extents[i].DiskNumber);
        }

        CloseHandle(hVolume);
        return true;
    }

    /**
     * Retrieves the properties of the specified disk.
     * @param diskNumber The number of the disk (e.g., 0 for PhysicalDrive0).
     * @param serialNumber A string to store the disk serial number.
     * @param model A string to store the disk model.
     * @param size A LARGE_INTEGER to store the disk size in bytes.
     * @return true if the operation was successful, otherwise false.
     */
    static bool GetDiskProperties(int diskNumber, std::string& serialNumber, std::string& model, LARGE_INTEGER& size) {
        std::wstring devicePath = L"\\\\.\\PhysicalDrive" + std::to_wstring(diskNumber);
        HANDLE hDevice = CreateFileW(devicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open device. Error: " << GetLastError() << std::endl;
            return false;
        }

        STORAGE_PROPERTY_QUERY query;
        ZeroMemory(&query, sizeof(query));
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        BYTE buffer[1024];
        ZeroMemory(buffer, sizeof(buffer));
        DWORD bytesReturned = 0;

        if (!DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer, sizeof(buffer), &bytesReturned, NULL)) {
            std::cerr << "Failed to query device properties. Error: " << GetLastError() << std::endl;
            CloseHandle(hDevice);
            return false;
        }

        STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer;

        // Serial Number
        if (deviceDescriptor->SerialNumberOffset != 0) {
            char* serialNumberOffset = (char*)buffer + deviceDescriptor->SerialNumberOffset;
            size_t serialLength = 0;
            while (serialNumberOffset[serialLength] != '\0' && serialLength < sizeof(buffer)) {
                ++serialLength;
            }
            serialNumber = std::string(serialNumberOffset, serialLength);
        }

        // Model
        if (deviceDescriptor->ProductIdOffset != 0) {
            char* modelOffset = (char*)buffer + deviceDescriptor->ProductIdOffset;
            size_t modelLength = 0;
            while (modelOffset[modelLength] != '\0' && modelLength < sizeof(buffer)) {
                ++modelLength;
            }
            model = std::string(modelOffset, modelLength);
        }

        // Size
        DISK_GEOMETRY dg;
        if (!DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &bytesReturned, NULL)) {
            std::cerr << "Failed to get disk geometry. Error: " << GetLastError() << std::endl;
            CloseHandle(hDevice);
            return false;
        }
        size.QuadPart = dg.Cylinders.QuadPart * dg.TracksPerCylinder * dg.SectorsPerTrack * dg.BytesPerSector;

        CloseHandle(hDevice);
        return true;
    }
};

class DiskPropertiesFetcher {
private:
    std::string serialNumber;
    std::string model;
    LARGE_INTEGER size;

public:
    /**
     * Constructor for DiskPropertiesFetcher.
     * @param volumePath The path to the volume (e.g., L"\\\\.\\C:").
     */
    DiskPropertiesFetcher(const std::wstring& volumePath)
        : serialNumber(""), model(""), size({ 0 }) {  // Initialize members
        std::vector<int> diskNumbers;
        if (DiskUtils::GetVolumeDiskExtents(volumePath, diskNumbers)) {
            for (int diskNumber : diskNumbers) {
                if (DiskUtils::GetDiskProperties(diskNumber, serialNumber, model, size)) {
                    return;
                }
            }
        }
    }

    /**
     * Retrieves the disk serial number.
     * @return The serial number as a std::string.
     */
    std::string DiskSerialNumber() const {
        return serialNumber;
    }

    /**
     * Retrieves the disk model.
     * @return The model as a std::string.
     */
    std::string DiskModel() const {
        return model;
    }

    /**
     * Retrieves the disk size in gigabytes.
     * @return The size in gigabytes as a DWORD.
     */
    DWORD DiskSize() const {
        return static_cast<DWORD>(size.QuadPart / (1024 * 1024 * 1024));  // Convert to GB
    }
};

#endif // DISKUTILSLIB_H
