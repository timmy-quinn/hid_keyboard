import pcbnew 
import sys
import os

source_layer_names = ["User.Drawings", "User.Eco1", "User.Edge_Cuts"]
target_layer_name = "User.9"

def get_layer_id(board, layer_name):
    return board.GetLayerID(layer_name)


def get_board(path =""):
    if path == "":
        board = pcbnew.GetBoard()
        if(board is None):
            raise ValueError("board not found")
    else: 
        print(f"loading board from path {path}") 
        board = pcbnew.LoadBoard(path)
    return board

def copy_lines(board): 
    # Define source layers (you can add or change layers)
    source_layer_names = ["User.Drawings", "User.Eco1", "User.Edge_Cuts"]
    source_layers = [get_layer_id(board, name) for name in source_layer_names] 
    print(source_layers)

    # Define the target layer
    target_layer = get_layer_id(board, target_layer_name)

    # Track the number of copied lines
    copied_count = 0

    # Iterate through all drawings (graphic items) on the PCB
    for drawing in board.GetDrawings():
        print("got drawing")
        if isinstance(drawing, pcbnew.PCB_SHAPE):
            print("is instance of a drawing")
            print(f"drawing layer: {drawing.GetLayer()}")
            if drawing.GetLayer() in source_layers:
                print("copying line")
                # Create a new shape with the same properties
                new_line = pcbnew.PCB_SHAPE(board)
                new_line.SetShape(drawing.GetShape())
                new_line.SetLayer(target_layer)
                new_line.SetWidth(drawing.GetWidth())
                new_line.SetStart(drawing.GetStart())
                new_line.SetEnd(drawing.GetEnd())

                # Add the new line to the board
                board.Add(new_line)
                copied_count += 1

    #Set the board to modified. 
    pcbnew.SetModified(True)
    pcbnew.Refresh()
    print(f"✅ Copied {copied_count} lines to {pcbnew.GetBoard().GetLayerName(target_layer)} layer.")
    pcbnew.SaveBoard("test_output.kicad_pcb", board)


def copy_footprint_graphics(board, source_layer_names, target_layer_name):
    # Convert layer names to layer IDs
    source_layers = [get_layer_id(board, name) for name in source_layer_names]
    target_layer = get_layer_id(board, target_layer_name)
    copied_count = 0

    # Iterate over all footprints
    for footprint in board.GetFootprints():
        print(f"Processing footprint: {footprint.GetReference()}")

        # Iterate over footprint graphic elements (lines, arcs, circles, polygons)
        for drawing in footprint.GraphicalItems():
            if isinstance(drawing, pcbnew.PCB_SHAPE) and drawing.GetLayer() in source_layers:
                # Create a new independent shape
                new_shape = pcbnew.PCB_SHAPE(board)
                new_shape.SetShape(drawing.GetShape())
                new_shape.SetLayer(target_layer)
                new_shape.SetWidth(drawing.GetWidth())

                new_shape.SetStart(drawing.GetStart())
                new_shape.SetEnd(drawing.GetStart())
                board.Add(new_shape)
                copied_count += 1

    # Refresh PCB view
    print(f"✅ Copied {copied_count} graphical elements from footprints to {board.GetLayerName(target_layer)} layer.")

def set_line_width(board, target_layer_name, width):
    for drawing in board.GetDrawings(): 
        if isinstance(drawing, pcbnew.PCB_SHAPE):
            print("is instance of a drawing")
            print(f"drawing layer: {drawing.GetLayer()}")
            if drawing.GetLayer() == get_layer_id(board, target_layer_name):
                drawing.SetWidth(width)                


def main(): 
    if len(sys.argv) < 3:
        raise ValueError("Please provide the path to the board and the the name of the new file")   
    board_file = sys.argv[1]
    new_file_name = sys.argv[2]
    new_file_path = new_file_name + ".kicad_pcb"
    if os.path.exists(new_file_path):
        os.remove(new_file_path)
        print("Removed pre-existing file")    
    
    board = get_board(board_file) 
    copy_footprint_graphics(board, source_layer_names, target_layer_name)
    set_line_width(board, "User.9", 0)
    board.SetModified()
    pcbnew.SaveBoard(new_file_path, board)
    pcbnew.Refresh()

if __name__ == "__main__":
    main()
