import socket
import threading
from protocol import ProtocolMessage, MessageType
from placement import place_ships, generate_coordinates, is_valid_coordinate
from utils import print_board, create_empty_board
import time

HOST = '54.173.19.229' # poner tu direccion propia de ip dependiendo de la instancia que se abra
PORT = 8080

# Inicializa los tableros
own_board = create_empty_board()
enemy_board = create_empty_board()

game_over = False
my_turn = False

def send_message(sock, msg: ProtocolMessage):
    sock.sendall((msg.to_string() + "\n").encode())

def receive_messages(sock):
    global enemy_board, game_over, my_turn
    while True:
        try:
            raw = sock.recv(1024).decode().strip()
            if not raw:
                print("❌ Desconectado del servidor.")
                game_over = True
                break

            msg = ProtocolMessage.from_string(raw)
            if msg.type == MessageType.HIT:
                if my_turn:
                    print("🔥 ¡Impacto!")
                    cell = msg.data[0]
                    enemy_board[cell] = 'X'
                else:
                    print("🔥 ¡Impacto en tu barco!")
                    cell = msg.data[0]
                    own_board[cell] = 'X'
                print_board(enemy_board, "Tablero Enemigo", False)    
                print_board(own_board, "Tu Tablero", True)
                if my_turn:
                    my_turn = False
            elif msg.type == MessageType.MISS:
                if my_turn:
                    print("💧 Fallo.")
                    cell = msg.data[0]
                    enemy_board[cell] = 'O'
                else:
                    print("💧 Fallo en tu barco.")
                    cell = msg.data[0]
                    own_board[cell] = 'O'
                print_board(enemy_board, "Tablero Enemigo", False)
                print_board(own_board, "Tu Tablero", True)
                if my_turn:
                    my_turn = False
            elif msg.type == MessageType.SUNK:
                if my_turn:
                    print("💥 ¡Barco hundido del enemigo!")
                    cell = msg.data[0]
                    enemy_board[cell] = 'X'
                else:
                    print("💥 ¡Tu barco ha sido hundido!")
                    cell = msg.data[0]
                    own_board[cell] = 'X'
                print_board(enemy_board, "Tablero Enemigo", False)
                print_board(own_board, "Tu Tablero", True)
                if my_turn:
                    my_turn = False
            elif msg.type == MessageType.WIN:
                print("🏆 ¡Has ganado!")
                game_over = True
                break
            elif msg.type == MessageType.LOSE:
                print("💀 Has perdido.")
                game_over = True
                break
            elif msg.type == MessageType.TURN:
                my_turn = True
                print("🎯 ¡Es tu turno!")
            elif msg.type == MessageType.CHAT:
                print("[Chat] " + ",".join(msg.data))
            else:
                print(f"[Mensaje recibido] {msg.to_string()}")
        except Exception as e:
            print(f"❌ Error recibiendo mensaje: {e}")
            game_over = True
            break


def main():
    global own_board, game_over, my_turn

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.settimeout(5)  # tiempo de espera para intentar conectar
        try:
            sock.connect((HOST, PORT))
            print("✅ Conectado al servidor.")
        except (socket.timeout, ConnectionRefusedError, OSError) as e:
            print(f"❌ No se pudo conectar al servidor en {HOST}:{PORT}.")
            print(f"Detalles del error: {e}")
            return  # salir del programa si no hay conexión

        # Establece timeout de 60 segundos para el socket
        sock.settimeout(60)

        # Espera en Loop mientras se le une a una ROOM
        while True:
            try:
                raw = sock.recv(1024).decode().strip()
                if not raw:
                    print("❌ Desconectado del servidor.")
                    return
                msg = ProtocolMessage.from_string(raw)
                if msg.type == MessageType.REGISTER:
                    print(f"📦 Te has unido a la sala: {msg.data[0]}")
                    break
            except Exception as e:
                print(f"❌ Error recibiendo mensaje: {e}")
                continue

        # Fase de colocación de barcos
        own_board, all_ship_positions, ships_list = place_ships(own_board)
        placed_cells = list(own_board.keys())

        print("🛳️ Barcos colocados en el tablero:")
        print_board(own_board)

        input("Presiona ENTER cuando estés listo para comenzar el juego.")
        send_message(sock, ProtocolMessage(MessageType.READY, str(ships_list)))

        # Espera en Loop mientras oponente pone LISTO
        while True:
            try:
                raw = sock.recv(1024).decode().strip()
                if not raw:
                    print("❌ Desconectado del servidor.")
                    return
                msg = ProtocolMessage.from_string(raw)
                if msg.type == MessageType.READY:
                    print(f"📦 El oponente está listo. Comienza el juego.")
                    break
            except Exception as e:
                print(f"❌ Error recibiendo mensaje: {e}")
                continue

        # Inicia el hilo para recibir mensajes
        threading.Thread(target=receive_messages, args=(sock,), daemon=True).start()

        # Loop principal de juego (turnos)
        while not game_over:
            if my_turn:
                try:
                    cell = input("📍 Tu turno. Coordenada a disparar (ej: A5): ").upper()
                    if len(cell) < 2 or not cell[0].isalpha() or not cell[1:].isdigit():
                        print("❗ Formato inválido.")
                        continue
                    send_message(sock, ProtocolMessage(MessageType.FIRE, [cell]))
                    print(f"📍 Disparando a {cell}...")
                except (KeyboardInterrupt, EOFError):
                    print("👋 Saliendo del juego...")
                    game_over = True
                    break
            else:
                time.sleep(1)


    except Exception as e:
        print(f"❌ Ocurrió un error inesperado: {e}")

    finally:
        try:
            sock.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()
