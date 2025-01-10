import time
import sys
import threading
import random
import pygame
import serial
from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from PIL import Image

###
#INTIALIZATION OF SOUND#
pygame.init()
pygame.mixer.music.set_volume(1.0)
#INITIALIZATION OF MATRIX DISPLAY#
options = RGBMatrixOptions()
options.rows = 32
options.cols = 64
options.chain_length = 1
options.parallel = 1
options.gpio_slowdown = 3
options.hardware_mapping = 'adafruit-hat' #adafruit-hat
options.pwm_bits = 6 #Remove if causing issues
matrix = RGBMatrix(options = options)
canvas = matrix.CreateFrameCanvas()
font = graphics.Font()
###

###
#HIGHSCORE GET#
highscore = 0
f = open("highscore.txt", "r")
try:
    highscore = int(f.readline())
except:
    highscore = 0
f.close()

###

###
#USB COMMUNICATION#
coin_door_port = ""
led_driver_port = ""
score_driver_port = ""

def check_serial_port(port):
    global coin_door_port, led_driver_port, score_driver_port
    try:
        ser = serial.Serial(port=port, baudrate=9600, timeout=1)
        data = ""
        while data not in [b'COIN_DOOR', b'LED_DRIVE', b'SCOR_DRIV']:
            if(ser.in_waiting > 0):
                data = ser.read(9)
                print(data)
                if(data == b'COIN_DOOR'):
                    coin_door_port = port
                elif(data == b'LED_DRIVE'):
                    led_driver_port = port
                elif(data == b'SCOR_DRIV'):
                    score_driver_port = port
            time.sleep(0.1)
        ser.close()
    except Exception as e:
        print(f"USB Detection Error: {e}")
    return
###

game_gifs_list = ["/home/pinball/Desktop/Pinball/game_gifs/GradeA.gif", "/home/pinball/Desktop/Pinball/game_gifs/GradeB.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/GradeC.gif", "/home/pinball/Desktop/Pinball/game_gifs/GradeD.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/GradeF.gif", "/home/pinball/Desktop/Pinball/game_gifs/Blessing.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/Blizzard.gif", "/home/pinball/Desktop/Pinball/game_gifs/Cloudy.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/Hurricane.gif", "/home/pinball/Desktop/Pinball/game_gifs/HeartTurn.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/Heart.gif", "/home/pinball/Desktop/Pinball/game_gifs/Internship.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/Rainy.gif", "/home/pinball/Desktop/Pinball/game_gifs/ScholarshipLoss.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/ScholarshipReceive.gif", "/home/pinball/Desktop/Pinball/game_gifs/SISManLaser.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/Snow.gif", "/home/pinball/Desktop/Pinball/game_gifs/Sunny.gif",
                "/home/pinball/Desktop/Pinball/game_gifs/WheelLoss.gif", "/home/pinball/Desktop/Pinball/game_gifs/WheelWin.gif", 
                "/home/pinball/Desktop/Pinball/game_gifs/FightLose.gif", "/home/pinball/Desktop/Pinball/game_gifs/FightWin.gif", "/home/pinball/Desktop/Pinball/game_gifs/Windy.gif"]

##
#GLOBAL VARIABLES#
deposited_money = 0
price = 0.75 #set to desired price
score = 0
debug_mode = 0
start_button_pressed = 0
volume = 1.0
##

##
#Multi-threading#
lock = threading.Lock()
stop_event = threading.Event()
##

def coindoor(port):
    global deposited_money, price, start_button_pressed, volume, debug_mode, ser
    data = ""
    d = ""
    try:
        with lock:
            ser = serial.Serial(port=port, baudrate=9600, timeout=1)
    except Exception as e:
        print(f"Error opening serial port {port}: {e}")
        return
    while not stop_event.is_set():
    
        while(data != b'COIN_DOOR'):
            if(ser.inWaiting() > 0):
                data = ser.read(9)
            else:
                time.sleep(0.05)
        if(ser.in_waiting > 0):
            d = ser.read(4).decode('utf-8')
        else:
            time.sleep(0.05)
        with lock:
            if d in ["0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"]:
                if(int(d[0])) == 1:
                    deposited_money += 0.25
                if(int(d[1])) == 1:
                    if(volume >= 1):
                        pygame.mixer.music.set_volume(1)
                    else:
                        volume = max(0, volume - 0.05)
                        pygame.mixer.music.set_volume(volume)
                if(int(d[2])) == 1:
                    if(volume <= 0):
                        pygame.mixer.music.set_volume(0)
                    else:
                        volume = min(1, volume + 0.05)
                        pygame.mixer.music.set_volume(volume)
                if(int(d[3])) == 1:
                    debug_mode = 1
        time.sleep(0.1)
    return

def main():
    lives = 3
    
    #gameplay logic
    
    if(lives == 0):
        end_game()
    return

def end_game():
    global score, highscore
    stop_music(0)
    play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Air.ogg", -1)
    score_display(5, (255, 0, 0))
    if(int(score) > int(highscore)):
        two_row_text("NEW", "HIGHSCORE!", display_time=5)
        highscore = score
        f = open("highscore.txt", "w")
        f.write(highscore)
        f.close()
    else:
        two_row_text("GAME", "OVER!", display_time=5)
    highscore_display(10)
    scrolling_text(["CREATED BY", "RYAN DONNELLAN", "SPECIAL THANKS", "ALEXA SCHUSTER", "RHEA VURGHESE", "KYLE WILT"])
    idle_loop()
    return

def idle_loop():
    global deposited_money, price, start_button_pressed, volume, debug_mode
    
    stop_music(0)
    
    while(True):
        play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Air.ogg", -1)
        if(deposited_money == 0 and price != 0):
            dvd_text_effect("INSERT COINS", (255, 0, 0), 30)
            time.sleep(0.1)
            play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Anthem.ogg")
            gif_display("startup.gif", 5, 25)
            time.sleep(1)
            stop_music(2000)
            time.sleep(2)
            play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Fixer.ogg")
            waiting_for_money_display(20)
            time.sleep(1)
            gif_display(random.choice(game_gifs_list), random.randint(2, 5), 12)
            stop_music(1000)
            time.sleep(1)
        elif(deposited_money > 0 and price != 0):
            while(deposited_money > 0):
                waiting_for_money_display(20)
                time.sleep(1)
                gif_display("/home/pinball/Desktop/Pinball/startup.gif", random.randint(0, 1), 25)
                time.sleep(1)
            stop_music(1000)
            time.sleep(1)
        if(deposited_money >= price or price == 0):
            if(start_button_pressed == 1):
                deposited_money = deposited_money - price
                start_button_pressed = 0
                main()
    return

def waiting_for_money_display(display_time, font_color=(255, 0, 0)):
    global canvas, price, deposited_money
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    textColor = graphics.Color(font_color[0], font_color[1], font_color[2])
    
    start = time.time()
    if(price == 0):
        text1 = "FREE PLAY!"
    elif(deposited_money == 0):
        text1 = "INSERT COINS"
    else:
        text1 = "CREDIT: " + str(format(deposited_money, ".2f"))
    if(deposited_money >= price):
        text2 = "PRESS START!"
    else:
        text2 = "PRICE: " + str(format(price, ".2f"))
    
    while(True):
        canvas.Clear()
        line1 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(text1) - 1) // 2)), 7, textColor, text1)
        line2 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(text2) - 1) // 2)), 21, textColor, text2)
        canvas = matrix.SwapOnVSync(canvas)
        end = time.time()
        if((end - start) >= display_time):
            break

def dvd_text_effect(text="insert coins", font_color=(255, 0, 0), play_time=30):
    global canvas
    start = time.time()
    canvas.Clear()
    x = 0
    y = 8
    dirX = dirY = 1
    speed = 1
    
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    textColor = graphics.Color(font_color[0], font_color[1], font_color[2])
    
    text_height = 5
    text_width = (4 * len(text)) + (len(text) - 1)
    
    screen_height = canvas.height + 2 #added +2
    screen_width = canvas.width + 2 #added +2
    
    while(True):
        canvas.Clear()
        line = graphics.DrawText(canvas, font, x, y, textColor, text)
        time.sleep(0.1)
        canvas = matrix.SwapOnVSync(canvas)
        if(y + text_height >= screen_height or y < (text_height + 2)): #changed "text_height" to "text_height + 1"
            dirY = dirY * (-1)
            textColor = graphics.Color(random.randint(50, 255), random.randint(50, 255), random.randint(50, 255))
        if(x + text_width >= screen_width or x < 0):
            dirX = dirX * (-1)
            textColor = graphics.Color(random.randint(50, 255), random.randint(50, 255), random.randint(50, 255))
        
        x += dirX * speed
        y += dirY * speed
        
        end = time.time()
        if((end - start) >= play_time):
            break
        elif((end - start) >= play_time - 1):
            stop_music(1000)
    return

def scrolling_text(text=["THANKS"], font_color=(255, 0, 0)):
    global canvas
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    textColor = graphics.Color(font_color[0], font_color[1], font_color[2])
    
    for item in text:
        new_item = item.split(" ")
        pos = canvas.height - 1
        
        for x in range(canvas.height + 6):
            canvas.Clear()
            line1 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(new_item[1]) - 1) // 2)), pos + 6, textColor, new_item[1])
            line2 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(new_item[0]) - 1) // 2)), pos, textColor, new_item[0])
            pos -= 1
            if((pos + line1 + 6) < -4):
                pos = canvas.width
            time.sleep(0.12)
            canvas = matrix.SwapOnVSync(canvas)
    return

def two_row_text(text1="DEBUG", text2="DEBUG", display_time=10, font_color=(255, 0, 0)):
    global canvas
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    changing_color = 0
    textColor = graphics.Color(font_color[0], changing_color, font_color[2])

    start = time.time()
    while(True):
        canvas.Clear()
        line1 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(str(text1)) - 1) // 2)), 7, textColor, str(text1))
        line2 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(str(text2)) - 1) // 2)), 21, textColor, str(text2))
        canvas = matrix.SwapOnVSync(canvas)
        end = time.time()
        if((end - start) >= display_time):
            break
        elif((end-start) >= 1):
            if(changing_color <= 200):
                changing_color += 4
            else:
                changing_color = 0
            textColor = graphics.Color(255, changing_color, 0)
    return

def play_music(music_dir, loop_count=0):
    pygame.mixer.music.load(music_dir)
    pygame.mixer.music.play(loops=loop_count)
    return
    
def stop_music(fade_out_ms=0):
    pygame.mixer.music.fadeout(fade_out_ms)
    
    while pygame.mixer.music.get_busy():
        pygame.time.Clock().tick(10) 
    return

def gif_display(gif_dir, loop_count=0, fr=17):
    global canvas
    canvas.Clear()
    gif = Image.open(gif_dir)
    num_frames = gif.n_frames

    playback_count = 0
    
    while(playback_count <= loop_count):
        for frame_index in range(0, num_frames):
            gif.seek(frame_index)
            frame = gif.copy()
            #canvas.Clear() #Maybe introduce this back if there's still issues?
            canvas.SetImage(frame.convert("RGB"))
            matrix.SwapOnVSync(canvas, framerate_fraction=fr)
        playback_count += 1
        canvas.Clear()

    gif.close()
    return

def gif_display_sound(gif_dir, music_dir, loop_count=0, fr=17):
    global canvas
    canvas.Clear()
    gif = Image.open(gif_dir)
    num_frames = gif.n_frames
    play_music(music_dir)
    playback_count = 0
    
    while(playback_count <= loop_count):
        for frame_index in range(0, num_frames):
            gif.seek(frame_index)
            frame = gif.copy()
            #canvas.Clear() #Maybe introduce this back if there's still issues?
            canvas.SetImage(frame.convert("RGB"))
            matrix.SwapOnVSync(canvas, framerate_fraction=fr)
        playback_count += 1
    
    stop_music()
    gif.close()
    canvas.Clear()
    return

def score_display(display_time, font_color=(255, 165, 0)):
    global canvas, score
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    textColor = graphics.Color(font_color[0], font_color[1], font_color[2])
    
    score_x = abs(32 - ((5 * len("SCORE:") - 1) // 2))
    
    start = time.time()
    while(True):
        canvas.Clear()
        line1 = graphics.DrawText(canvas, font, score_x, 7, textColor, "SCORE:")
        line2 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(str(score).zfill(9)) - 1) // 2)), 21, textColor, str(score).zfill(9))
        canvas = matrix.SwapOnVSync(canvas)
        end = time.time()
        if((end - start) >= display_time):
            break
    return

def highscore_display(display_time):
    global canvas, highscore
    font.LoadFont("/home/pinball/Desktop/Pinball/donnell2.bdf")
    changing_color = 0
    textColor = graphics.Color(255, changing_color, 0)
    score_x = abs(32 - ((5 * len("HIGHSCORE:") - 1) // 2))
    
    start = time.time()
    while(True):
        canvas.Clear()
        line1 = graphics.DrawText(canvas, font, score_x, 7, textColor, "HIGHSCORE:")
        line2 = graphics.DrawText(canvas, font, abs(32 - ((5 * len(str(highscore).zfill(9)) - 1) // 2)), 21, textColor, str(highscore).zfill(9))
        canvas = matrix.SwapOnVSync(canvas)
        end = time.time()
        if((end - start) >= display_time):
            break
        elif((end-start) >= 1):
            if(changing_color <= 200):
                changing_color += 4
            else:
                changing_color = 0
            textColor = graphics.Color(255, changing_color, 0)
    return

if __name__ == '__main__':
    for p in ["/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3"]:
        check_serial_port(p)
    
    print([coin_door_port, led_driver_port, score_driver_port])

    time.sleep(0.1)

    stop_event.clear()
    bg_thread1 = threading.Thread(target=coindoor, args=(coin_door_port,))
    bg_thread1.start()
    
    gif_display_sound("/home/pinball/Desktop/Pinball/startup.gif", "startup.wav", 0, 25)
    gif_display_sound("/home/pinball/Desktop/Pinball/forge_splash.gif", "forge_splash.ogg", 0, 14)
    play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Fixer.ogg")
    scrolling_text(["CREATED BY", "RYAN DONNELLAN", "SPECIAL THANKS", "ALEXA SCHUSTER", "RHEA VURGHESE", "KYLE WILT"])
    stop_music(250)
    
    if(debug_mode == 1):
        dvd_text_effect("DEBUG ACTIVE", (255, 0, 0), 5)
        for x in game_gifs_list:
            gif_display(x, 1, 12)
            time.sleep(0.25)
        debug_mode = 0
        end_game()
    else:
        idle_loop()