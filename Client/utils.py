import string

def print_board(board, title="Tablero", show_ships=True):
    """
    Imprime un tablero 10x10 desde un diccionario de celdas.
    `show_ships=True` permite mostrar barcos (para tablero propio).
    """
    rows = list(string.ascii_uppercase[:10])
    cols = list(map(str, range(1, 11)))

    print(f"\n=== {title} ===")
    header = "   " + " ".join(f"{c:>2}" for c in cols)
    print(header)

    for row in rows:
        line = f"{row} "
        for col in cols:
            cell = f"{row}{col}"
            symbol = "."
            if cell in board:
                content = board[cell]
                if content == "X":
                    symbol = "X"
                elif content == "O":
                    symbol = "O"
                elif show_ships:
                    symbol = content  # Mostrar inicial del barco si está habilitado
            line += f" {symbol:>2}"
        print(line)

def update_board(board, cell, result, is_own_board=False):
    """
    Actualiza el diccionario del tablero con el resultado de un disparo.
    - board: dict del tablero
    - cell: string "A5"
    - result: "HIT" / "MISS"
    - is_own_board: True si es el tablero propio (entonces se ve si fue un HIT enemigo)
    """
    if result == "HIT":
        board[cell] = "X"
    elif result == "MISS":
        board[cell] = "O"
    # (para tablero propio también se puede marcar con "X" donde acertaron)

def create_empty_board():
    """
    Retorna un diccionario vacío con el formato del tablero.
    """
    board = {}
    rows = list(string.ascii_uppercase[:10])
    cols = list(map(str, range(1, 11)))

    for row in rows:
        for col in cols:
            cell = f"{row}{col}"
            board[cell] = "."
    return board

def save_board_to_file(board, filename, title="Tablero", show_ships=True):
    """
    Guarda el tablero en un archivo de texto.
    """
    rows = list(string.ascii_uppercase[:10])
    cols = list(map(str, range(1, 11)))

    with open(filename, 'w') as f:
        f.write(f"=== {title} ===\n")
        header = "   " + " ".join(f"{c:>2}" for c in cols)
        f.write(header + "\n")

        for row in rows:
            line = f"{row} "
            for col in cols:
                cell = f"{row}{col}"
                symbol = "."
                if cell in board:
                    content = board[cell]
                    if content == "B":
                        symbol = "B" if show_ships else "."
                    elif content == "X":
                        symbol = "X"
                    elif content == "O":
                        symbol = "O"
                line += f" {symbol:>2}"
            f.write(line + "\n")
            

    