
# 💥 HugeStressTest

**HugeStressTest** is a C++ application designed to perform heavy load testing by generating a large number of HTTP requests concurrently. It can be used to benchmark servers, test stability, and measure response times under stress.

## ⚙️ Features

- Concurrent HTTP request generation
- Configurable target URL and number of requests
- Measures response times and success/failure rates
- Simple console output with progress and results

## 📁 Project Structure

```
HugeStressTest/
├── main.cpp           # Main program file
├── StressTest.h       # Header for stress test class
├── StressTest.cpp     # Implementation of stress test logic
└── README.md          # This documentation file
```

## 🚀 Build Instructions

1. Make sure you have a C++ compiler installed (GCC, Clang, MSVC).
2. Compile the program:

```bash
g++ main.cpp StressTest.cpp -o HugeStressTest -std=c++11 -pthread
```

3. Run the executable:

```bash
./HugeStressTest
```

## 🧪 Usage

- When running, the program will prompt you to enter the target URL and number of requests.
- It will then start sending requests concurrently and display progress.
- After completion, summary statistics will be shown.

## 📦 Dependencies

- Standard C++ libraries
- Thread support (`-pthread` flag on Linux/macOS)

## 📄 License

This project currently has no specified license. Use it freely for learning and testing purposes. Contact the author for commercial or other uses.

## 👤 Author

- GitHub: [SigmaGit1eR](https://github.com/SigmaGit1eR)

---
