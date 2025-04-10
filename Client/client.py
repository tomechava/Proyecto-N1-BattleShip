import socket
import threading
from protocol import ProtocolMessage, MessageType
from placement import place_ships
from utils import print_board, create_empty_board, log_to_file  # 👈 NUEVO
import time

HOST = '3.83.154.199'
PORT = 8080

own_board = create_empty_board()
enemy_board = create_empty_board()

game_over = False
my_turn = False

def send_message(sock, msg: ProtocolMessage):
    message_str = msg.to_string()
    log_to_file(f"Enviando mensaje: {message_str}")  # 👈 Log enviado
    sock.sendall((message_str + "\n").encode())

def receive_messages(sock):
    global enemy_board, game_over, my_turn
    while True:
        try:
            raw = sock.recv(1024).decode().strip()
            if not raw:
                log_to_file("Desconectado del servidor.")  # 👈 Log desconexión
                print("❌ Desconectado del servidor.")
                game_over = True
                break

            log_to_file(f"Recibido mensaje: {raw}")  # 👈 Log recibido
            msg = ProtocolMessage.from_string(raw)

            if msg.type == MessageType.HIT:
                cell = msg.data[0]
                if my_turn:
                    print("🔥 ¡Impacto!")
                    enemy_board[cell] = 'X'
                else:
                    print("🔥 ¡Impacto en tu barco!")
                    own_board[cell] = 'X'
                my_turn = False
            elif msg.type == MessageType.MISS:
                cell = msg.data[0]
                if my_turn:
                    print("💧 Fallo.")
                    enemy_board[cell] = 'O'
                else:
                    print("💧 Fallo en tu barco.")
                    own_board[cell] = 'O'
                my_turn = False
            elif msg.type == MessageType.SUNK:
                cell = msg.data[0]
                if my_turn:
                    print("💥 ¡Barco hundido del enemigo!")
                    enemy_board[cell] = 'X'
                else:
                    print("💥 ¡Tu barco ha sido hundido!")
                    own_board[cell] = 'X'
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

            print_board(enemy_board, "Tablero Enemigo", False)
            print_board(own_board, "Tu Tablero", True)

        except Exception as e:
            error_msg = f"❌ Error recibiendo mensaje: {e}"
            print(error_msg)
            log_to_file(error_msg)
            game_over = True
            break

def main():
    global own_board, game_over, my_turn

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        sock.settimeout(5)
        try:
            sock.connect((HOST, PORT))
            print("✅ Conectado al servidor.")
            log_to_file("Conectado al servidor.")
        except (socket.timeout, ConnectionRefusedError, OSError) as e:
            err = f"No se pudo conectar a {HOST}:{PORT}. Error: {e}"
            print(f"❌ {err}")
            log_to_file(err)
            return

        sock.settimeout(60)

        # Esperando sala
        while True:
            try:
                raw = sock.recv(1024).decode().strip()
                if not raw:
                    print("❌ Desconectado del servidor.")
                    log_to_file("Desconectado durante espera de sala.")
                    return
                log_to_file(f"Recibido mensaje: {raw}")
                msg = ProtocolMessage.from_string(raw)
                if msg.type == MessageType.REGISTER:
                    sala = msg.data[0]
                    print(f"📦 Te has unido a la sala: {sala}")
                    log_to_file(f"Te has unido a la sala: {sala}")
                    break
            except Exception as e:
                log_to_file(f"Error esperando sala: {e}")
                continue

        # Colocación de barcos
        own_board, _, ships_list = place_ships(own_board)
        print("🛳️ Barcos colocados en el tablero:")
        print_board(own_board)
        log_to_file("Barcos colocados.")

        input("Presiona ENTER cuando estés listo para comenzar el juego.")
        send_message(sock, ProtocolMessage(MessageType.READY, f"{ships_list}"))

        # Esperando READY del oponente
        while True:
            try:
                raw = sock.recv(1024).decode().strip()
                if not raw:
                    log_to_file("Desconectado esperando READY.")
                    return
                log_to_file(f"Recibido mensaje: {raw}")
                msg = ProtocolMessage.from_string(raw)
                if msg.type == MessageType.READY:
                    print("📦 El oponente está listo. Comienza el juego.")
                    log_to_file("Oponente listo. Juego iniciado.")
                    break
            except Exception as e:
                log_to_file(f"Error esperando READY: {e}")
                continue

        # Recibiendo mensajes del juego
        threading.Thread(target=receive_messages, args=(sock,), daemon=True).start()

        # Bucle de juego
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
                    log_to_file("Jugador interrumpió el juego manualmente.")
                    print("👋 Saliendo del juego...")
                    game_over = True
                    break
            else:
                time.sleep(1)

    except Exception as e:
        err = f"❌ Error inesperado: {e}"
        print(err)
        log_to_file(err)

    finally:
        try:
            sock.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()
