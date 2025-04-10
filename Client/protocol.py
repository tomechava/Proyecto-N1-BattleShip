from enum import Enum

class MessageType(Enum):
    REGISTER = "REGISTER"
    READY = "READY"
    FIRE = "FIRE"
    HIT = "HIT"
    MISS = "MISS"
    SUNK = "SUNK"
    WIN = "WIN"
    LOSE = "LOSE"
    DISCONNECT = "DISCONNECT"
    PLACE = "PLACE"
    UNKNOWN = "UNKNOWN"
    TURN = "TURN"

class ProtocolMessage:
    def __init__(self, type_: MessageType, data: list):
        self.type = type_
        self.data = data

    def to_string(self):
        return f"{self.type.value}|{','.join(self.data)}"
    
    def to_serialized_string(self):
        if self.type == MessageType.READY and all(isinstance(ship, list) for ship in self.data):
            # Serializa cada barco como "A1-A2-A3" y los separa con ","
            flat_ships = ['-'.join(ship) for ship in self.data]
            return f"{self.type.value}|{','.join(flat_ships)}"
        else:
            return self.to_string()

    def __str__(self):
        return self.to_string()

    @staticmethod
    def from_string(raw):
        if '|' not in raw:
            return ProtocolMessage(MessageType.UNKNOWN, [])
        
        type_str, *rest = raw.strip().split('|')
        try:
            type_enum = MessageType(type_str)
        except ValueError:
            type_enum = MessageType.UNKNOWN

        data = rest[0].split(',') if rest else []
        return ProtocolMessage(type_enum, data)

# ✅ Esta función facilita la creación rápida de mensajes
def create_message(message_type: MessageType, data: list) -> str:
    return ProtocolMessage(message_type, data).to_
