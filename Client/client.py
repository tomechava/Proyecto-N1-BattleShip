import socket
import threading
from protocol import ProtocolMessage, MessageType
from placement import place_ships, generate_coordinates, is_valid_coordinate
from utils import print_board, create_empty_board

HOST = '127.0.0.1'
PORT = 12345

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

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        print("Conectado al servidor.")

        # Fase de colocación de barcos
        own_board = place_ships()
        placed_cells = list(own_board.keys())
        
        print("Barcos colocados en el tablero:")
        print_board(own_board)

        input("Presiona ENTER cuando estés listo para comenzar el juego.")
        send_message(sock, ProtocolMessage(MessageType.READY, [own_board]))

        # Inicia el hilo para recibir mensajes
        threading.Thread(target=receive_messages, args=(sock,), daemon=True).start()

        # Loop de turnos del jugador
        while True:
            try:
                cell = input("📍 Coordenada a disparar (ej: A5): ").upper()
                if len(cell) < 2 or not cell[0].isalpha() or not cell[1:].isdigit():
                    print("Formato inválido.")
                    continue
                send_message(sock, ProtocolMessage(MessageType.FIRE, [cell]))
            except (KeyboardInterrupt, EOFError):
                print("Saliendo del juego...")
                break


    
