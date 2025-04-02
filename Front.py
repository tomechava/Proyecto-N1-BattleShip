# Prueba de conexion a servidor de AWS no es front real (esto se cambia directamente cuando hagamos el real)


import socket

# Dirección IP y puerto del servidor
HOST = '34.239.119.250'  # Cambia esto por la IP pública de tu servidor EC2
PORT = 8080

# Crear el socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Conectar al servidor
client_socket.connect((HOST, PORT))

# Recibir la respuesta del servidor
data = client_socket.recv(1024)
print('Mensaje del servidor:', data.decode())

# Cerrar la conexión
client_socket.close()
