# This program allows you to easily configure the lights for the LED Driver and get the 12 bytes you want in hex
# On the board, the LED #'s count starting from the Left side of the board, going down and then moving right.

# All you need to do is put in your desired colors for the LEDs, and then add a 13th byte in the MSB position:
# EXAMPLE:  this program returns '0x0000000000000000000000E0', 
#           the LED driver would want something like: '0xFF0000000000000000000000E0', where FF is the brightness
#           (also note that FF for brightness causes a breathing effect, anything other than FF is interpreted as a literal brightness value)

def pack_leds(led_colors):   
    bitstream = 0
    
    for x in led_colors:
        for y in x:
            bitstream = (bitstream << 1) | y
    
    output = bitstream.to_bytes(12, 'little')
    return ''.join(f'{byte:02X}' for byte in output)

leds = [        
#   [R, G, B]
    [1, 1, 1],  # LED 1
    [0, 0, 0],  # LED 2
    [0, 0, 0],  # LED 3
    [0, 0, 0],  # LED 4
    [0, 0, 0],  # LED 5
    [0, 0, 0],  # LED 6
    [0, 0, 0],  # LED 7
    [0, 0, 0],  # LED 8
    [0, 0, 0],  # LED 9
    [0, 0, 0],  # LED 10
    [0, 0, 0],  # LED 11
    [0, 0, 0],  # LED 12
    [0, 0, 0],  # LED 13
    [0, 0, 0],  # LED 14
    [0, 0, 0],  # LED 15
    [0, 0, 0],  # LED 16
    [0, 0, 0],  # LED 17
    [0, 0, 0],  # LED 18
    [0, 0, 0],  # LED 19
    [0, 0, 0],  # LED 20
    [0, 0, 0],  # LED 21
    [0, 0, 0],  # LED 22
    [0, 0, 0],  # LED 23
    [0, 0, 0],  # LED 24
    [0, 0, 0],  # LED 25
    [0, 0, 0],  # LED 26
    [0, 0, 0],  # LED 27
    [0, 0, 0],  # LED 28
    [0, 0, 0],  # LED 29
    [0, 0, 0],  # LED 30
    [0, 0, 0],  # LED 31
    [0, 0, 0],  # LED 32
]

print(pack_leds(leds))