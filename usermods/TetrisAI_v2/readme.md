# Tetris AI effect usermod

This usermod brings you a effect brings a self playing Tetris game. The mod needs version 0.14 or above as it is based on matrix support. The effect was tested on an ESP32 with a WS2812B 16x16 matrix.

Version 1.0

## Installation 

Just activate the usermod with `-D USERMOD_TETRISAI` and the effect will become available under the name 'Tetris AI'.

## Usage

It is best to set the background color to black, the border color to light grey and the game over color (foreground) to dark grey.

### Sliders and boxes

#### Sliders

* speed: speed the game plays
* look ahead: how many pieces is the AI allowed to know the next pieces (0 - 2)
* intelligence: how good the AI will play
* Rotate color: make the colors shift (rotate) every few cicles
* Mistakes free: how many good moves between mistakes (if activated)

#### Checkboxes

* show next: if true a space of 5 pixels from the right is used to show the next pieces. The whole segment is used for the grid otherwise.
* show border: if true an additional column of 1 pixel is used to draw a border between the grid and the next pieces
* mistakes: if true the worst instead of the best move is choosen every few moves (read above)

## Best results

 If the speed is set to be a little bit faster than a good human could play with maximal intelligence and very few mistakes it makes people furious/happy at a party.