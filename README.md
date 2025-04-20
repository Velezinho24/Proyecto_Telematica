# Proyecto_Telematica

## 🧾 Descripción General

Este proyecto consiste en la implementación de un **servidor web compatible con HTTP/1.1** como parte del curso de **Telemática**. Utiliza la **API de Berkeley Sockets**, y permite atender múltiples clientes concurrentemente mediante hilos (`threads`). El servidor es capaz de procesar solicitudes HTTP con los métodos `GET`, `HEAD` y `POST`.

---

## ⚙️ Modo de Ejecución

El servidor se ejecuta con el siguiente comando:

```bash
./server <HTTP_PORT> <LOG_FILE> <DOCUMENT_ROOT>
```

# Ejemplo:

```bash
./server 8080 logs/server.log www
```

<HTTP_PORT>: Puerto donde el servidor escucha conexiones (ej. 8080).

<LOG_FILE>: Archivo donde se registran las peticiones y respuestas.

<DOCUMENT_ROOT>: Carpeta donde se encuentran los recursos web (HTML, imágenes, etc).

---

📁 Estructura de Archivos

1. server.cpp — Implementación del Servidor
Este archivo contiene la lógica completa del servidor web.

🧩 Funcionalidades:


Escucha conexiones entrantes en el puerto especificado.

Procesa solicitudes HTTP/1.1 válidas (GET, HEAD, POST).

Entrega archivos estáticos desde una carpeta raíz (DOCUMENT_ROOT).

Manejo de errores HTTP: 400 Bad Request, 404 Not Found, 204 No Content, etc.

Soporte concurrente: atiende múltiples clientes al mismo tiempo usando std::thread.

Logger: registra en un archivo los eventos importantes (peticiones, respuestas, errores).

🧠 Principales componentes:


handleClient(): procesa cada conexión entrante.

serveFile(): lee y entrega archivos desde el sistema de archivos.

sendResponse(): construye y envía la respuesta HTTP al cliente.

getMimeType(): determina el tipo MIME del archivo solicitado.

logPrintf(): registra eventos con marcas de tiempo en log.txt.

sigintHandler(): cierra ordenadamente el servidor al presionar Ctrl+C.

2. log.txt — Registro del Servidor (Logger)
Este archivo contiene una traza de todas las actividades realizadas por el servidor.

📝 ¿Qué se registra?
Inicio y apagado del servidor.

Cada solicitud entrante: método, recurso, cuerpo (en POST), código de respuesta.

Longitud del contenido servido y errores.

---

🧪 Ejemplos:

```text

2025-04-20 11:29:21 POST body: Hola mundo desde POST
2025-04-20 11:29:21 Responded 200 OK

2025-04-20 11:35:58 Method: GET | Path: /caso1_imagen/index.html | Status: 200 OK | Content-Length: 230
2025-04-20 11:36:16 Method: GET | Path: /caso1_imagen/index.ht | Status: 404 Not Found | Content-Length: 48
```

---

🧪 Casos de Prueba
✅ Funcionalidades probadas:
✔️ GET /archivo.html (devuelve página solicitada)

✔️ HEAD /archivo.html (solo encabezados)

✔️ POST /archivo.html (procesa y muestra cuerpo enviado)

✔️ Entrega de imágenes y videos

✔️ Errores 404 y 400 cuando corresponde

✔️ Registro detallado en log.txt

🔧 Herramientas utilizadas:
curl, Postman, Firefox

🧪 Comandos de prueba:
```bash
curl -X POST http://localhost:8080/test.html -d "mensaje de prueba"
curl http://localhost:8080/caso1_imagen/index.html
```