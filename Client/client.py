import socket
import threading
from protocol import ProtocolMessage, MessageType
from placement import place_ships, generate_coordinates, is_valid_coordinate
from utils import print_board, create_empty_board

HOST = '54.166.91.45' # poner tu direccion propia de ip dependiendo de la instancia que se abra
PORT = 8080

# Inicializa los tableros
own_board = create_empty_board()
enemy_board = create_empty_board()

def send_message(sock, msg: ProtocolMessage):
    sock.sendall((msg.to_string() + "\n").encode())

def receive_messages(sock):
    global enemy_board
    while True:
        try:
            raw = sock.recv(1024).decode().strip()
            if not raw:
                print("Desconectado del servidor.")
                break

            msg = ProtocolMessage.from_string(raw)
            if msg.type == MessageType.HIT:
                print("🔥 ¡Impacto!")
                cell = msg.data[0]
                enemy_board[cell] = 'X'
                print_board(enemy_board, "Enemigo", False)
            elif msg.type == MessageType.MISS:
                print("💨 Fallaste.")
                cell = msg.data[0]
                enemy_board[cell] = 'O'
                print_board(enemy_board, "Enemigo", False)
            elif msg.type == MessageType.WIN:
                print("🏆 ¡Has ganado!")
                break
            elif msg.type == MessageType.LOSE:
                print("💀 Has perdido.")
                break
            elif msg.type == MessageType.CHAT:
                print("[Chat] " + ",".join(msg.data))
            else:
                print(f"[Mensaje recibido] {msg.to_string()}")
        except Exception as e:
            print(f"Error recibiendo mensaje: {e}")
            break


def main():
    global own_board

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(5)  # tiempo de espera para intentar conectar
            try:
                sock.connect((HOST, PORT))
                print("✅ Conectado al servidor.")
            except (socket.timeout, ConnectionRefusedError, OSError) as e:
                print(f"❌ No se pudo conectar al servidor en {HOST}:{PORT}.")
                print(f"Detalles del error: {e}")
                return  # salir del programa si no hay conexión
            
            # Inicia el hilo para recibir mensajes
            threading.Thread(target=receive_messages, args=(sock,), daemon=True).start()
            
            #Espera en Loop mientras se le une a una ROOM
            while True:
                try:
                    raw = sock.recv(1024).decode().strip()
                    if not raw:
                        print("Desconectado del servidor.")
                        break
                    msg = ProtocolMessage.from_string(raw)
                    if msg.type == MessageType.REGISTER:
                        print(f"📦 Te has unido a la sala: {msg.data[0]}")
                        break
                except Exception as e:
                    print(f"Error recibiendo mensaje: {e}")
                    break

            # Fase de colocación de barcos
            own_board, all_ship_positions, ships_list = place_ships(own_board)
            placed_cells = list(own_board.keys())
            
            print("🛳️ Barcos colocados en el tablero:")
            print_board(own_board)

            input("Presiona ENTER cuando estés listo para comenzar el juego.")
            send_message(sock, ProtocolMessage(MessageType.READY, ships_list))


            # Loop de turnos del jugador
            while True:
                try:
                    cell = input("📍 Coordenada a disparar (ej: A5): ").upper()
                    if len(cell) < 2 or not cell[0].isalpha() or not cell[1:].isdigit():
                        print("❗ Formato inválido.")
                        continue
                    send_message(sock, ProtocolMessage(MessageType.FIRE, [cell]))
                except (KeyboardInterrupt, EOFError):
                    print("👋 Saliendo del juego...")
                    break
    except Exception as e:
        print(f"❌ Ocurrió un error inesperado: {e}")

if __name__ == "__main__":
    main()
