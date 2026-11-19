# Camera Calibration

Programa de visión artificial desarrollado en C++ con OpenCV para calibrar una cámara y corregir la distorsión de imágenes y vídeos.

## Funcionamiento

La calibración utiliza varias imágenes de un tablero de ajedrez tomadas desde diferentes posiciones y orientaciones.

El programa detecta las esquinas interiores del tablero, refina sus coordenadas y calcula los parámetros de la cámara.

Se generan los siguientes resultados:

- Matriz de parámetros intrínsecos.
- Coeficientes de distorsión.
- Error de reproyección.
- Resolución de las imágenes utilizadas.

Los parámetros se guardan en un archivo XML o YAML y pueden utilizarse posteriormente para corregir imágenes y vídeos.

## Programas incluidos

- `calibrate` calcula los parámetros de calibración.
- `undistort` corrige la distorsión utilizando una calibración guardada.

## Requisitos

- Compilador compatible con C++11.
- CMake 3.10 o posterior.
- OpenCV 4.6 o posterior.

## Compilación

Desde la raíz del proyecto:

```bash
cmake -S . -B build
cmake --build build
```

Los siguientes ejemplos se ejecutan desde el directorio `build/`:

```bash
cd build
```

## Calibración con imágenes

Introduce las fotografías del tablero en `data/` y ejecuta:

```bash
./calibrate --size=0.03 --rows=5 --cols=6 \
  calibracion.yml \
  ../data/tablero_01.png \
  ../data/tablero_02.png \
  ../data/tablero_03.png
```

Ajusta estos parámetros a tu tablero:

- `--size` indica el lado de cada casilla. En el ejemplo se utilizan metros, por lo que `0.03` representa 3 cm.
- `--rows` indica el número de filas de casillas.
- `--cols` indica el número de columnas de casillas.

Un tablero de 5 filas y 6 columnas contiene 4 × 5 esquinas interiores.

Se necesitan al menos dos vistas válidas. Para obtener una calibración más fiable, utiliza varias fotografías con diferentes posiciones y orientaciones, manteniendo la misma resolución.

### Modo interactivo

Añade `--verbose` para visualizar las detecciones y ajustar los parámetros de búsqueda:

```bash
./calibrate --verbose --size=0.03 --rows=5 --cols=6 \
  calibracion.yml ../data/tablero_*.png
```

Durante la selección de vistas:

- **Enter** incorpora una vista con el tablero detectado.
- **Espacio** continúa sin incorporarla.
- **Esc** termina la captura y calibra con las vistas seleccionadas.

## Calibración con cámara

```bash
./calibrate --camera=0 --verbose \
  --size=0.03 --rows=5 --cols=6 calibracion.yml
```

El índice `0` identifica la primera cámara. Puede cambiarse según el dispositivo.

## Corrección de una imagen

```bash
./undistort calibracion.yml ../data/imagen.png imagen_corregida.png
```

El programa muestra la imagen original y la corregida. Pulsa una tecla distinta de **Esc** para guardar el resultado.

## Corrección de un vídeo

```bash
./undistort --video --fourcc=MJPG \
  calibracion.yml ../data/entrada.avi video_corregido.avi
```

La opción `--fourcc` permite seleccionar el códec de salida.

Utiliza imágenes o vídeos de la misma cámara y con la resolución empleada durante la calibración.

## Ayuda

```bash
./calibrate --help
./undistort --help
```

## Datos

El directorio `data/` se utiliza para almacenar imágenes, vídeos y archivos de calibración necesarios para las pruebas. Las rutas de entrada se indican al ejecutar cada programa.
