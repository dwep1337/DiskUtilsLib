# DiskUtilsLib

DiskUtilsLib is a C++ library for retrieving disk properties on Windows systems, including serial number, model, and size.

## Features

- **Retrieve Disk Serial Number**
- **Retrieve Disk Model**
- **Retrieve Disk Size**

## How to Download

1. **Clone the Repository**:
 https://github.com/dwep1337/DiskUtilsLib.git

2. **Include the Header**

   Add `DiskUtilsLib.hpp` to your project and include it in your source files:

   ```cpp
   #include "DiskUtilsLib.hpp"
   ```

## Usage Example

Here is a simple example of how to use the library to fetch disk properties:

```cpp
#include <iostream>
#include "DiskUtilsLib.hpp"

int main() {
    std::wstring volumePath = L"\\\\.\\C:";  // Change this to the volume path you want to check, e.g., L"\\\\.\\D:"

    DiskPropertiesFetcher fetcher(volumePath);

    std::cout << "Serial Number: " << fetcher.DiskSerialNumber() << std::endl;
    std::cout << "Model: " << fetcher.DiskModel() << std::endl;
    std::cout << "Size: " << fetcher.DiskSize() << " GB" << std::endl;

    return 0;
}
```
