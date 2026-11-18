#pragma once

// Framework de pruebas minimo, sin dependencias externas.
// Cada TEST_CASE se registra automaticamente y se ejecuta desde test_main.cpp.
// No pretende competir con Catch2 o GoogleTest: solo demuestra que un pipeline
// puede ejecutar pruebas reales con CERO dependencias adicionales.

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

struct TestCase {
  std::string name;
  std::function<void()> fn;
};

struct Registrar {
  Registrar(std::string name, std::function<void()> fn);
};

void reportFailure(const char* file, int line, const std::string& message);
int runAll();

template <typename Actual, typename Expected>
void checkEqual(const Actual& actual, const Expected& expected, const char* expression,
                const char* file, int line) {
  if (!(actual == expected)) {
    std::ostringstream message;
    message << expression << " -> obtenido: " << actual << ", esperado: " << expected;
    reportFailure(file, line, message.str());
  }
}

template <typename Actual, typename Expected>
void checkNotEqual(const Actual& actual, const Expected& expected, const char* expression,
                   const char* file, int line) {
  if (actual == expected) {
    std::ostringstream message;
    message << expression << " -> ambos valores son " << actual;
    reportFailure(file, line, message.str());
  }
}

} // namespace testing

#define TEST_CASE(name)                                      \
  static void name();                                        \
  static ::testing::Registrar registrar_##name(#name, name); \
  static void name()

#define CHECK(expression)                                                        \
  do {                                                                           \
    if (!(expression)) {                                                         \
      ::testing::reportFailure(__FILE__, __LINE__, "CHECK fallo: " #expression); \
    }                                                                            \
  } while (false)

#define CHECK_EQ(actual, expected) \
  ::testing::checkEqual((actual), (expected), #actual " == " #expected, __FILE__, __LINE__)

#define CHECK_NE(actual, expected) \
  ::testing::checkNotEqual((actual), (expected), #actual " != " #expected, __FILE__, __LINE__)

#define CHECK_THROWS_AS(expression, exceptionType)                                             \
  do {                                                                                         \
    bool threw = false;                                                                        \
    try {                                                                                      \
      static_cast<void>(expression);                                                           \
    } catch (const exceptionType&) {                                                           \
      threw = true;                                                                            \
    } catch (...) {                                                                            \
      ::testing::reportFailure(__FILE__, __LINE__, "se lanzo otra excepcion en " #expression); \
    }                                                                                          \
    if (!threw) {                                                                              \
      ::testing::reportFailure(__FILE__, __LINE__,                                             \
                               "no se lanzo " #exceptionType " en " #expression);              \
    }                                                                                          \
  } while (false)
