# battleship_client/placement.py

import string

BOARD_SIZE = 10
LETTERS = string.ascii_uppercase[:BOARD_SIZE]

SHIP_RULES = {
    "Portaaviones": 5,
    "Buque de guerra": 4,
    "Crucero 1": 3,
    "Crucero 2": 3,
    "Destructor 1": 2,
    "Destructor 2": 2,
    "Submarino 1": 1,
    "Submarino 2": 1,
    "Submarino 3": 1,
}


def is_valid_coordinate(coord):
    if len(coord) < 2 or len(coord) > 3:
        return False
    row, col = coord[0], coord[1:]
    return row in LETTERS and col.isdigit() and 1 <= int(col) <= BOARD_SIZE


def generate_coordinates(start, end):
    start_row, start_col = start[0], int(start[1:])
    end_row, end_col = end[0], int(end[1:])

    coords = []
    if start_row == end_row:
        for col in range(min(start_col, end_col), max(start_col, end_col) + 1):
            coords.append(f"{start_row}{col}")
    elif start_col == end_col:
        for row in LETTERS[min(LETTERS.index(start_row), LETTERS.index(end_row)): max(LETTERS.index(start_row), LETTERS.index(end_row)) + 1]:
            coords.append(f"{row}{start_col}")
    else:
        return []  # Diagonal no permitido
    return coords


def place_ships(board):
    print("Coloca tus barcos. Tablero de 10x10, filas A-J y columnas 1-10.")
    placed_cells = set()
    all_ship_positions = []

    for ship_name, ship_size in SHIP_RULES.items():
        while True:
            print(f"\nColoca el {ship_name} ({ship_size} casillas):")
            start = input("  Coordenada inicial (ej. A1): ").upper()
            end = input("  Coordenada final (ej. A5): ").upper()

            if not is_valid_coordinate(start) or not is_valid_coordinate(end):
                print("  ❌ Coordenadas inválidas.")
                continue

            coords = generate_coordinates(start, end)

            if len(coords) != ship_size:    # Verifica el tamaño del barco
                print(f"  ❌ El {ship_name} debe ocupar exactamente {ship_size} casillas.")
                continue

            if any(cell in placed_cells for cell in coords):    # Verifica si las coordenadas ya están ocupadas
                print("  ❌ Algunas coordenadas ya están ocupadas.")
                continue

            placed_cells.update(coords) # Actualiza las celdas ocupadas
            all_ship_positions.extend(coords)   # Guarda las posiciones del barco
            board_with_ships = update_board_graphically(board, coords, ship_name)  # Actualiza el tablero gráficamente
            print(f"  ✅ {ship_name} colocado.")    #
            break

    return board_with_ships, all_ship_positions

def update_board_graphically(board, coords, ship_name):
    for cell in coords:
        board[cell] = ship_name[0]  # Coloca la inicial del barco
    return board

def main():
    board = {f"{row}{col}": "." for row in LETTERS for col in range(1, BOARD_SIZE + 1)}
    placed_board, all_ship_positions = place_ships(board)
    
    print("\nTablero final:")
    for cell, value in placed_board.items():
        print(f"{cell}: {value}")

    print("\nPosiciones de los barcos:")
    print(", ".join(all_ship_positions))
    
if __name__ == "__main__":
    main()
