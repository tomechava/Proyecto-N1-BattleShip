# Proyecto-N1-BattleShip

### Tomas Echavarria, Manuel Arenas.

# VIDEO EXPLICATIVO

Link video:

# INTRODUCCIÓN
## Sobre el proyecto
Este proyecto consiste en la implementación del clásico juego de Batalla Naval en una arquitectura cliente-servidor. El servidor está desarrollado en C++ utilizando sockets TCP para permitir la conexión simultánea de múltiples clientes, mientras que el cliente está construido en Python con una interfaz por consola. El objetivo principal fue permitir partidas en línea entre dos jugadores, gestionando la colocación de barcos, los turnos de juego y la validación de disparos a través de un protocolo de comunicación personalizado. Además, se buscó diseñar una estructura modular, robusta y extensible, aplicando principios de concurrencia, sincronización y comunicación en red.

# DESARROLLO
## Diagramas UML
### Server
![SERVER_UML](https://github.com/user-attachments/assets/97f33d6a-9d40-4dda-b81a-b25b774ee9c4)

### Client
![CLIENT_UML](https://github.com/user-attachments/assets/2115058b-544f-468a-9c04-11e3f6291d62)

### Diagrama de secuencia UML
![SEQUENCE](https://github.com/user-attachments/assets/a11047f0-323c-4520-aeca-f95b005899c9)

# ASPECTOS LOGRADOS Y NO LOGRADOS
## Aspectos logrados
* Se implementó un servidor multicliente en C++ que permite conexiones concurrentes mediante hilos.

* Se creó un sistema de emparejamiento que forma salas de juego automáticamente cuando hay dos jugadores disponibles.

* Se desarrolló un protocolo personalizado compatible con TCP/IP para la comunicación entre cliente y servidor.

* Se implementó la lógica completa de colocación de barcos por parte de ambos jugadores, respetando las reglas oficiales del juego.

* Se programó el manejo de turnos, permitiendo que los jugadores disparen de forma alternada.

* Se construyó una clase Room que maneja la interacción completa entre los dos jugadores de una partida.

* Se desarrolló un cliente en Python que permite jugar desde consola, incluyendo la visualización del tablero propio y enemigo.

* Se integró una estructura modular con archivos separados para protocolo, utilidades gráficas del tablero, y la lógica del cliente.

* Se logró una comunicación funcional entre cliente y servidor para los comandos básicos como PLACE, READY y FIRE.

## Aspectos no logrados
* No se implementó el temporizador de 30 segundos por turno.

* El servidor aún no detecta correctamente los impactos (HIT) y (SUNK) cuando se recibe un comando FIRE.

* No se logra compilar en el servidor ubuntu, Error desconocido, compila correctamente de forma local (saltandose el uso de la IP y claramente la conexion al servidor).

# CONCLUSIONES

En este proyecto se logró implementar un servidor multicliente para el juego Batalla Naval, con emparejamiento automático, manejo por turnos y comunicación a través de un protocolo personalizado sobre TCP/IP. Elegir un protocolo claro y compatible con TCP fue clave para garantizar una comunicación confiable y ordenada entre clientes y servidor, usando sockets tipo stream que se adaptan bien a este tipo de juegos en tiempo real.

Además, se demostró la importancia de aplicar medidas de seguridad al aceptar conexiones, evitando accesos no deseados y asegurando la estabilidad del servidor. En resumen, se construyó una base sólida que permite partidas interactivas entre jugadores, lista para futuras mejoras como interfaz gráfica o expansión a nuevas plataformas.

# REFERENCIAS

* https://csperkins.org/teaching/2007-2008/networked-systems/lecture04.pdf

