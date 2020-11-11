# JSON protocol

Viewer draw primitives. Each primitive has field `type`

Type may be one of:
 - circle
 - rectangle
 - triangle
 - polyline
 - message
 - popup
 - options
 - end
 
All primitives will be shown **after** `type: end` comes, thus closing current frame and start next one.
 
For positions used array-like format, e.g. `[x1,y1,x2,y2,...]`

For colors used integer format in form of `ARGB`. 
But zero alpha component means fully opaque object instead of invisible one. 
Example:
```
0xff0000    # red
0xffff0000  # exactly the same as above, but aplha set to 255 manually (fully opaque)
0x7700ff00  # transparent green
0x0000ff    # blue
0x000033    # dark blue
```

See "options" section to learn about viewer state and layer manipulation

### circle
```yaml
type: 'circle'
p: [float, float]   # center position (x, y)
r: float            # radius
color: color        # color, integer format
fill: boolean       # whenever to fill with color
```

### rectangle
```yaml
type: 'rectangle'
tl: [float, float]                  # top-left point
br: [float, float]                  # bottom-right point
color: color                        # color, integer format
       [color, color, color, color] #  or you may specify different color for each vertex 
                                    #    (field should be either color or array of colors)
                                    #  vertex order for colors is top_left, bottom_left, top_right, bottom_right  
```
:information_source: If you mix up `tl` and `br` positions they will be normalized

### triangle
```yaml
type: 'triangle'
points: [float, float, float, 
         float, float, float]  # exactly 3 points (6 floats)
color: color                   # color, integer format
       [color, color, color]   #   or you may specify different colors for each vertex
                               #    (field should be either color or array of colors)
fill: boolean                  # whenever to fill with color
```

### polyline
```yaml
type: 'polyline'
points: [float, float,       
         float, float, ...]  # arbitrary points count, but at least 4 float (2 points)
color: color                 # color, integer format
```

### message
Messages will be shown in `Frame message` window inside viewer.
You can send several messages during one frame. Newline added automatically after each message
Escape characters also allowed, e.g. you can send `multilined\nstring`
```yaml
type: 'message'
message: string
```
### popup
Show popup window with message when cursor overlaps with given shape

Round popup:
```yaml
type: 'popup'
p: [float, float]   # circle center position
r: float            # circle radius
text: string        # text to be displayed (escape characters as \n are allowed)
```

Rectangular popup:
```yaml
type: 'popup'
tl: [float, float]  # top-left
br: [float, float]  # bottom-right point
text: string        # text to be displayed (escape characters as \n are allowed)
```

### options
Frame options
- `layer` Set active layer (1-10) for primitives.
_Note:_ `end` primitive reset layer to its default value
- `permanent` Primitives from permanent frame drawn before each frame, thus allow to once draw unchanged data, like map border
```yaml
type: 'options'
layer: integer      # set layer for primitives
permanent: boolean  # whenever to draw primitives to permanent layer
```
### end
Closes frame and starts new one
```yaml
type: 'end'
```
