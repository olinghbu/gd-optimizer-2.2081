# GD Optimizer: port a Geode v5 (GD 2.2081)

Port de GD Optimizer de Xanii (https://github.com/LXanii/GD-Optimizer), que era para Geode 2.0.0-beta.21 y GD 2.204.

## Cómo compilarlo

### Opción A: GitHub Actions (no hace falta instalar nada)
1. Crea un repositorio en GitHub y sube esta carpeta tal cual, incluida `.github/workflows/build.yml`.
2. Abre la pestaña **Actions**. La compilación arranca sola con cada push.
3. Cuando termine, descarga el artefacto **"GD Optimizer (Geode v5, GD 2.2081)"**. Dentro está `xanii.gd_optimizer.geode`.
4. Copia ese `.geode` a `Geometry Dash/geode/mods/`.

### Opción B: compilarlo en tu PC (Windows)
Necesitas Visual Studio 2022 (C++), CMake, la Geode CLI y el SDK (`geode sdk install` y luego `geode sdk install-binaries`).

```
geode build
```

## Cambios respecto al original
- mod.json: `geode` 5.9.0 y `gd` como objeto (`{"win": "2.2081"}`), como exige Geode v5.
- CMake con C++23 (los mods en C++20 ya no compilan en v5).
- Usa la API de Windows (`SetPriorityClass`) en lugar de `wmic`: no hay lag al entrar al nivel, no aparecen ventanas de consola y funciona en Windows 11 24H2+, donde `wmic` ya no existe.
- Al salir del nivel (o al cerrar el juego) devuelve cada programa a su prioridad original. El original los dejaba en "Baja" para siempre.
- Se ha corregido el error que impedía guardar el botón de activar/desactivar (antes se guardaba antes de que respondieras al popup).
- Ajustes nuevos: lista de programas editable, subir GD a prioridad Alta y restaurar al salir.
