import pcbnew
from enum import Enum

class lineType(Enum): 
    INTERIOR = 1
    EXTERIOR = 2
    CORNER = 2

class location(Enum): 
    TOP_C = 1
    TOP_R = 2
    TOP_L = 3
    BOT_C = 4
    BOT_R = 5
    BOT_L = 6
    LEFT = 7
    RIGHT = 8





def lineType(): 
    return false

def offsetBoardOutline(offset_mili):
    board = pcbnew.GetBoard()
    outline = board.GetBoardEdgesBoundingBox()

    offset_nano = offset_mili * (10^6)
    # Offset the outline by 0.5mm
    offset = pcbnew.VECTOR2I(offset_nano, offset_nano)  # 0.5mm in nanometers
    new_outline = outline.Inflate(offset.x, offset.y)

    # Update the board outline
    for edge in board.GetDrawings():
        if edge.GetLayer() == pcbnew.Edge_Cuts:
            edge.SetPosition(edge.GetPosition() + offset)
            print(f"Offsetting line by {offset_mili}")

    pcbnew.Refresh()

if __name__ == "__main__":
    offsetBoardOutline(10)

