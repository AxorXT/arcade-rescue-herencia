#include "test_framework.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace testing {
namespace {

std::vector<TestCase>& storage() {
  static std::vector<TestCase> cases;
  return cases;
}

int g_failures = 0;

} // namespace

Registrar::Registrar(std::string name, std::function<void()> fn) {
  storage().push_back({std::move(name), std::move(fn)});
}

void reportFailure(const char* file, int line, const std::string& message) {
  ++g_failures;
  std::cerr << "    [FALLO] " << file << ":" << line << " -> " << message << "\n";
}

int runAll() {
  std::cout << "Ejecutando " << storage().size() << " pruebas...\n\n";

  int failedTests = 0;
  for (const TestCase& test : storage()) {
    const int before = g_failures;
    std::cout << "[ RUN  ] " << test.name << "\n";
    try {
      test.fn();
    } catch (const std::exception& error) {
      reportFailure(__FILE__, __LINE__, std::string("excepcion no controlada: ") + error.what());
    } catch (...) {
      reportFailure(__FILE__, __LINE__, "excepcion desconocida");
    }

    if (g_failures == before) {
      std::cout << "[  OK  ] " << test.name << "\n";
    } else {
      ++failedTests;
      std::cout << "[ FALLO] " << test.name << "\n";
    }
  }

  const std::size_t total = storage().size();
  const std::size_t passed = total - static_cast<std::size_t>(failedTests);
  std::cout << "\nResultado: " << passed << "/" << total << " pruebas correctas";
  if (g_failures > 0) {
    std::cout << ", " << g_failures << " comprobaciones fallidas";
  }
  std::cout << "\n";

  return failedTests == 0 ? 0 : 1;
}

} // namespace testing

int main() {
  return testing::runAll();
}
