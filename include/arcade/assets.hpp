#pragma once

#include <string>

namespace arcade {

// Busca un recurso del juego (fuente, icono, sonidos) sin depender del
// directorio de trabajo desde el que se lance el ejecutable.
//
// Orden de busqueda:
//   1. explicitDirectory (si no esta vacio)
//   2. Variable de entorno ARCADE_RESCUE_ASSETS
//   3. "assets", "../assets", "../../assets"
//   4. <directorio del ejecutable>/assets
//   5. <directorio del ejecutable>/../share/arcade-rescue/assets
//
// Devuelve una cadena vacia si el recurso no existe. Nunca lanza excepciones:
// un asset ausente degrada la presentacion, no impide jugar.
std::string findAsset(const std::string& relativePath,
                      const std::string& explicitDirectory = std::string{});

// Directorio del ejecutable actual (cadena vacia si no se puede determinar).
std::string executableDirectory();

} // namespace arcade
