import socket
import time

SERVER_HOST = '127.0.0.1'  # o la IP pública si lo corres en AWS
SERVER_PORT = 5000         # Asegúrate de que sea el mismo que usa el servidor

# Coordenadas de los barcos (puedes mejorar esto con lógica más compleja o random)
barcos = ["A1", "A2", "A3", "B5", "C5", "D5", "E2", "E3", "E4", "E5"]

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((SERVER_HOST, SERVER_PORT))
    print("Conectado al servidor.")

    # Esperar mensaje de bienvenida (opcional)
    bienvenida = sock.recv(1024).decode()
    print(bienvenida.strip())

    # Enviar mensaje PLACE con las coordenadas de los barcos
    msg_place = "PLACE|" + ",".join(barcos)
    sock.sendall(msg_place.encode())
    print(f"Enviado: {msg_place}")

    time.sleep(1)  # pequeña espera para simular al usuario

    # Enviar mensaje READY
    sock.sendall("READY|".encode())
    print("Enviado: READY")

    # Escuchar mensajes del servidor (respuestas de juego)
    try:
        while True:
            data = sock.recv(1024)
            if not data:
                break
            print("Servidor dice:", data.decode().strip())
    except KeyboardInterrupt:
        print("Cliente interrumpido.")
    finally:
        sock.close()
        print("Conexión cerrada.")

if __name__ == "__main__":
    main()
