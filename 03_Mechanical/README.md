#Keyboard Case and Plate Notes

[Jo scotto video gasket mount ](https://www.youtube.com/watch?v=W4mXZvFGHDg&t=625s)

- uses 4mm gaskets on both top and bottom  
- There is a 3.7 mm gap between the top of the plate and case, and bottom of plate and case,
despite the fact that a 4mm gasket is being used. This allows for the gasket to squish the
 board in place. 
- There is a gap between the side of the plate and the case of 0.25mm

[Jo Scotto case and plate](https://www.youtube.com/watch?v=7azQkSu0m_U)

[anatomy of a keyboard article](https://matt3o.com/anatomy-of-a-keyboard/) 

[cherry switches developer notes](https://www.cherry.de/en-gb/gaming/developer)

def test_layers(board):
    for drawing in board.GetDrawings():
        print("got drawing")
        print(f"Found drawing fo type:{type(drawing)} on layer ID: {drawing.GetLayer()}")
### Dimensions of a keyboard plate and case 
- From the top of the plate to the top of the PCBs is 5.0mm <br>
- The PCB and plate (when using an FR-4 plate will both be 1.6mm) <br>
    - For a 3d printed plate the plate should be thicker, 3mm to 5mm
    - At the top and bottom of a key switch are the clips. Below the clips are
    two little outcroppings that clip to the underside of the plate. To enable
     space for these little outcroppings, you need to make sure this part of the
      plate is 1.5mm thick. The area of these clips and outcroppings is 6mm long 
      and less than 1mm in width. To be safe we can add an extra mm or two for 
      tolerance. An area 7mm by 1-1.5mm should be adequate. 
- The thickness of the hot swappable socket is 1.85mm <br>
- The switch cutout holes should be 14mm by 14mm squares
<br>
#### Total distances from top of plate to bottom of case: <br>
10.45mm = 5 (top of plate to pcb) + 1.6 (pcb) + 1.85mm (hot swap socket) + 3mm (tolerance) <br>
  

#### Threaded inserts
For the threaded inserts that I am using, the material thickness is 3.2 mm and the length is 5.0mm

#### Integrated plate  
Raise the edged of the "case" 6mm above the "plate"

