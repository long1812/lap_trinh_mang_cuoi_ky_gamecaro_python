#!/usr/bin/env python3
"""
client.py
Pygame GUI client for Caro (online via server.py)
Screens:
 - Login/Register
 - Lobby (list rooms, create/join/leave)
 - Game (board, chat, players, rank)

Controls:
 - Click board cell to send move
 - Type chat and press Enter to send
"""

import pygame, socket, threading, json, queue, sys, time

# Config
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 9009
BOARD_SIZE = 15
CELL = 32
MARGIN = 40
WINDOW_W = 1000
WINDOW_H = 700
FONT_NAME = None

# Networking: JSON per-line
class NetClient(threading.Thread):
    def __init__(self, host, port, incoming_q):
        super().__init__(daemon=True)
        self.host = host; self.port = port
        self.sock = None; self.incoming = incoming_q
        self.running = False
        self.lock = threading.Lock()

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # disable Nagle so small JSON lines are sent immediately
        try:
            self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        except Exception:
            pass
        self.sock.connect((self.host, self.port))
        self.running = True
        self.start()

    def run(self):
        buf = b""
        try:
            while self.running:
                data = self.sock.recv(4096)
                if not data:
                    break
                buf += data
                while b'\n' in buf:
                    line, buf = buf.split(b'\n',1)
                    try:
                        obj = json.loads(line.decode())
                        self.incoming.put(obj)
                    except Exception:
                        pass
        except Exception:
            pass
        finally:
            self.running = False
            self.incoming.put({"type":"disconnected"})

    def send(self, obj):
        try:
            data = json.dumps(obj, separators=(',',':')) + "\n"
            with self.lock:
                self.sock.sendall(data.encode())
        except Exception:
            pass

    def close(self):
        self.running = False
        try:
            self.sock.close()
        except Exception:
            pass

# Pygame GUI simple helpers
pygame.init()
screen = pygame.display.set_mode((WINDOW_W, WINDOW_H))
pygame.display.set_caption("Caro - Pygame Client")
font = pygame.font.SysFont(FONT_NAME, 18)
bigfont = pygame.font.SysFont(FONT_NAME, 28)

def draw_text(surf, text, x, y, color=(0,0,0), fontobj=font):
    surf.blit(fontobj.render(text, True, color), (x,y))

# App states
STATE_LOGIN = "login"
STATE_LOBBY = "lobby"
STATE_GAME = "game"

class App:
    def __init__(self):
        self.state = STATE_LOGIN
        self.incoming = queue.Queue()
        self.net = None
        self.nick = ""
        self.password = ""
        self.server = SERVER_HOST; self.port = SERVER_PORT
        self.rooms = []  # list of dicts
        self.selected_room_idx = 0
        self.room_name_input = ""
        self.room = None
        self.board = [[0]*BOARD_SIZE for _ in range(BOARD_SIZE)]
        self.turn = None
        self.players = []
        self.status = "idle"
        self.winner = None
        self.chat_lines = []
        self.chat_input = ""
        self.rank = {"wins":0,"losses":0}
        self.login_focus = "nick"
        self.confirm_surrender = False
        self.confirm_msg = "Are you sure?"
        self._load_local_creds()
        self.winner_popup_msg = None
        self.winner_popup_until = 0.0


    def _load_local_creds(self):
        try:
            with open("creds.txt","r") as f:
                lines = f.read().splitlines()
                if len(lines)>=2:
                    self.nick = lines[0]
                    self.password = lines[1]
        except Exception:
            pass
    def _save_local_creds(self):
        try:
            with open("client_creds.json","w") as f:
                json.dump({"nick":self.nick,"password":self.password}, f)
        except Exception:
            pass

    def connect_net(self):
        self.net = NetClient(self.server, self.port, self.incoming)
        try:
            self.net.connect()
            return True
        except Exception as e:
            print("connect failed", e)
            self.net = None
            return False

    def send(self, obj):
        if self.net:
            self.net.send(obj)

    def handle_incoming(self):
        while not self.incoming.empty():
            msg = self.incoming.get()
            t = msg.get("type")
            if t == "login_ok":
                self.nick = msg.get("nick")
                self.rank = msg.get("rank",{"wins":0,"losses":0})
                # popup confirm surrender
                self.confirm_surrender = False
                self.confirm_msg = "Are you sure?"
                self.state = STATE_LOBBY
                self._save_local_creds()
            elif t == "login_fail":
                self.append_chat("[SYSTEM] Login failed.")
            elif t == "register_ok":
                self.append_chat("[SYSTEM] Registered. Please login.")
                self._save_local_creds()
            elif t == "register_fail":
                self.append_chat("[SYSTEM] Register failed.")
            elif t == "identified":
                self.nick = msg.get("nick"); self.rank = msg.get("rank",{"wins":0,"losses":0}); self.state = STATE_LOBBY
            elif t == "rooms":
                self.rooms = msg.get("rooms",[])
            elif t == "room_joined":
                self.room = msg.get("room"); self.players = msg.get("players",[])
                self.append_chat(f"[SYSTEM] Joined {self.room}")
            elif t == "state":
                self.room = msg.get("room"); self.board = msg.get("board", self.board)
                self.turn = msg.get("turn"); self.players = msg.get("players",[])
                self.status = msg.get("status"); self.winner = msg.get("winner")
                if self.status == "won":
                    # append chat and show popup congratulation
                    winner = self.winner
                    if winner == self.nick:
                        text = "Congratulations! You have won!"
                    else:
                        text = f"Player {winner} has won. Congratulations!"
                    self.append_chat(f"[SYSTEM] Winner: {winner}")
                    # set popup for 3 seconds
                    self.winner_popup_msg = text
                    self.winner_popup_until = time.time() + 3.0
                elif self.status == "draw":
                    self.append_chat("[SYSTEM] Draw.")
                    self.winner_popup_msg = "Draw."
                    self.winner_popup_until = time.time() + 2.5
            elif t == "info":
                # server informational messages (e.g. surrender, end of match)
                m = msg.get("message","")
                if m:
                    self.append_chat(f"[INFO] {m}")
                    # show short popup for important info
                    if "lose" in m.lower() or "surrender" in m.lower() or "win" in m.lower():
                        self.winner_popup_msg = m
                        self.winner_popup_until = time.time() + 3.0
            elif t == "chat":
                frm = msg.get("from",""); text = msg.get("text","")
                self.append_chat(f"{frm}: {text}")
            elif t == "rank_update":
                self.rank = msg.get("rank", self.rank)
                self.append_chat(f"[SYSTEM] Rank updated: W{self.rank.get('wins',0)} L{self.rank.get('losses',0)}")
            elif t == "error":
                self.append_chat(f"[ERROR] {msg.get('message')}")
            elif t == "disconnected":
                self.append_chat("[SYSTEM] Disconnected from server")
                if self.net:
                    self.net.close()
                    self.net = None

    def append_chat(self, text):
        self.chat_lines.append(text)
        if len(self.chat_lines) > 200:
            self.chat_lines = self.chat_lines[-200:]

    def attempt_login(self):
        if not self.connect_net():
            self.append_chat("[SYSTEM] Cannot connect to server")
            return
        self.send({"type":"login","nick":self.nick,"password":self.password})

    def attempt_register(self):
        if not self.connect_net():
            self.append_chat("[SYSTEM] Cannot connect to server")
            return
        self.send({"type":"register","nick":self.nick,"password":self.password})

    # UI drawing functions for each state
    def draw_login(self):
        screen.fill((200,200,220))
        draw_text(screen, "Caro - Login / Register", 40, 20, fontobj=bigfont)
        draw_text(screen, "Nick:", 40, 100)
        nick_rect = pygame.Rect(120, 96, 200, 26)
        pygame.draw.rect(screen, (255,255,255), nick_rect)
        # highlight focus
        if self.login_focus == "nick":
            pygame.draw.rect(screen, (180,200,255), nick_rect, 2)
        draw_text(screen, self.nick, 126, 100)
        draw_text(screen, "Password:", 40, 140)
        pass_rect = pygame.Rect(120, 136, 200, 26)
        pygame.draw.rect(screen, (255,255,255), pass_rect)
        if self.login_focus == "password":
            pygame.draw.rect(screen, (180,200,255), pass_rect, 2)
        draw_text(screen, "*"*len(self.password), 126, 140)
        draw_text(screen, f"Server: {self.server}:{self.port}", 40, 190)

        pygame.draw.rect(screen, (100,200,100), (40, 240, 120, 34))
        draw_text(screen, "Login", 70, 248)
        pygame.draw.rect(screen, (100,150,255), (180, 240, 120, 34))
        draw_text(screen, "Register", 220, 248)

        y = 320
        draw_text(screen, "Messages:", 40, y)
        for i, line in enumerate(self.chat_lines[-6:]):
            draw_text(screen, line, 40, y+24 + i*20)

    def draw_lobby(self):
        screen.fill((230,230,230))
        draw_text(screen, f"Logged in as: {self.nick}  W:{self.rank.get('wins',0)} L:{self.rank.get('losses',0)}", 20, 10, fontobj=bigfont)

        pygame.draw.rect(screen, (255,255,255), (20, 60, 400, 420))
        draw_text(screen, "Rooms:", 30, 66)
        for i, r in enumerate(self.rooms):
            txt = f"{r.get('name')}  players:{r.get('players')}  {r.get('status')}"
            color = (0,0,0) if i!=self.selected_room_idx else (255,0,0)
            draw_text(screen, txt, 30, 96 + i*22, color)

        pygame.draw.rect(screen, (200,200,200), (450, 60, 230, 120))
        draw_text(screen, "Create room name:", 460, 66)
        pygame.draw.rect(screen, (255,255,255), (460, 92, 200, 28))
        draw_text(screen, self.room_name_input, 466, 96)
        pygame.draw.rect(screen, (100,200,100), (460, 130, 90, 36))
        draw_text(screen, "Create", 482, 138)
        pygame.draw.rect(screen, (100,150,255), (580, 130, 90, 36))
        draw_text(screen, "Join", 606, 138)
        pygame.draw.rect(screen, (255,180,80), (460, 180, 210, 36))
        draw_text(screen, "Refresh", 540, 188)

        pygame.draw.rect(screen, (200,120,120), (460, 230, 210, 36))
        draw_text(screen, "Disconnect", 520, 238)

        pygame.draw.rect(screen, (245,245,245), (20, 500, 900, 160))
        draw_text(screen, "Messages:", 30, 504)
        for i, line in enumerate(self.chat_lines[-6:]):
            draw_text(screen, line, 30, 526 + i*20)

    def draw_game(self):
        screen.fill((210,210,200))

        # Square-board layout (outer border thicker, inner lines thin)
        board_x0 = 20 + MARGIN
        board_y0 = 20 + MARGIN
        board_px = CELL * BOARD_SIZE
        board_rect = pygame.Rect(board_x0, board_y0, board_px, board_px)

        # Background for board
        pygame.draw.rect(screen, (245, 222, 179), board_rect)  # light wood-ish

        # Inner grid lines (thin)
        for i in range(1, BOARD_SIZE):
            x = board_x0 + i * CELL
            pygame.draw.line(screen, (0,0,0), (x, board_y0), (x, board_y0 + board_px), 1)
            y = board_y0 + i * CELL
            pygame.draw.line(screen, (0,0,0), (board_x0, y), (board_x0 + board_px, y), 1)

        # Outer border (thicker)
        pygame.draw.rect(screen, (0,0,0), (board_x0, board_y0, board_px, board_px), 4)

        # Draw X/O centered IN CELLS
        for r in range(BOARD_SIZE):
            for c in range(BOARD_SIZE):
                val = self.board[r][c]
                cx = board_x0 + c * CELL + CELL//2
                cy = board_y0 + r * CELL + CELL//2
                if val == 1:
                    off = int(CELL * 0.28)
                    pygame.draw.line(screen, (0,0,0), (cx-off, cy-off), (cx+off, cy+off), 2)
                    pygame.draw.line(screen, (0,0,0), (cx-off, cy+off), (cx+off, cy-off), 2)
                elif val == 2:
                    radius = int(CELL * 0.28)
                    pygame.draw.circle(screen, (0,0,0), (cx, cy), radius, 2)

        # INFO + CHAT panel (to the right)
        panel_x = board_x0 + board_px + 40
        pygame.draw.rect(screen, (240,240,240), (panel_x, 20, 400, 640))
        draw_text(screen, f"Room: {self.room}", panel_x+20, 30, fontobj=bigfont)
        draw_text(screen, f"Players: {', '.join(self.players)}", panel_x+20, 70)
        draw_text(screen, f"Turn: {self.turn}", panel_x+20, 100)
        draw_text(screen, f"Status: {self.status}", panel_x+20, 130)
        draw_text(screen, f"Rank: W{self.rank.get('wins',0)} L{self.rank.get('losses',0)}", panel_x+20, 160)

        # Chat area
        pygame.draw.rect(screen, (255,255,255), (panel_x+20, 200, 320, 360))
        for i, line in enumerate(self.chat_lines[-12:]):
            draw_text(screen, line, panel_x+26, 206 + i*26)

        # chat input & send
        pygame.draw.rect(screen, (255,255,255), (panel_x+20, 580, 260, 28))
        draw_text(screen, self.chat_input, panel_x+26, 584)
        pygame.draw.rect(screen, (100,200,100), (panel_x+300, 578, 80, 32))
        draw_text(screen, "Send", panel_x+320, 586)

        # control buttons
        pygame.draw.rect(screen, (200,120,120), (panel_x+20, 620, 120, 32))
        draw_text(screen, "Leave Room", panel_x+30, 628)
        pygame.draw.rect(screen, (180,180,255), (panel_x+160, 620, 120, 32))
        draw_text(screen, "Surrender", panel_x+200, 628)
        if self.confirm_surrender:
            # semi-transparent overlay
            overlay = pygame.Surface((WINDOW_W, WINDOW_H), pygame.SRCALPHA)
            overlay.fill((0,0,0,120))
            screen.blit(overlay, (0,0))
            # popup box
            pw, ph = 360, 140
            px = (WINDOW_W - pw)//2
            py = (WINDOW_H - ph)//2
            pygame.draw.rect(screen, (255,255,255), (px, py, pw, ph))
            pygame.draw.rect(screen, (0,0,0), (px, py, pw, ph), 2)
            draw_text(screen, self.confirm_msg, px+20, py+20, fontobj=bigfont)
            # Yes / No buttons
            pygame.draw.rect(screen, (200,80,80), (px+40, py+80, 100, 40))
            draw_text(screen, "yes", px+70, py+90)
            pygame.draw.rect(screen, (180,180,255), (px+220, py+80, 100, 40))
            draw_text(screen, "No", px+255, py+90)
            if getattr(self, "winner_popup_msg", None) and time.time() < getattr(self, "winner_popup_until", 0):
                overlay = pygame.Surface((WINDOW_W, WINDOW_H), pygame.SRCALPHA)
                overlay.fill((0,0,0,140))
                screen.blit(overlay, (0,0))
                pw = 520; ph = 120
                px = (WINDOW_W - pw)//2
                py = (WINDOW_H - ph)//2
                pygame.draw.rect(screen, (255,255,255), (px, py, pw, ph))
                pygame.draw.rect(screen, (0,0,0), (px, py, pw, ph), 2)
                # center message
                msg = self.winner_popup_msg
                draw_text(screen, msg, px+20, py+40, fontobj=bigfont)
        else:
            # clear expired
            if getattr(self, "winner_popup_until", 0) and time.time() >= getattr(self, "winner_popup_until", 0):
                self.winner_popup_msg = None
                self.winner_popup_until = 0

    def update(self):
        self.handle_incoming()

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                if self.net:
                    self.net.close()
                pygame.quit()
                sys.exit(0)

            if ev.type == pygame.KEYDOWN:
                if self.state == STATE_LOGIN:
                    if ev.key == pygame.K_BACKSPACE:
                        if self.login_focus == "nick":
                            self.nick = self.nick[:-1]
                        else:
                            self.password = self.password[:-1]
                    elif ev.key == pygame.K_RETURN:
                        self.attempt_login()
                    elif ev.key == pygame.K_TAB:
                        # toggle focus
                        self.login_focus = "password" if self.login_focus=="nick" else "nick"
                    
                    else:
                        ch = ev.unicode
                        if ch:
                            if self.login_focus == "nick":
                                self.nick += ch
                            else:
                                self.password += ch

                elif self.state == STATE_LOBBY:
                    if ev.key == pygame.K_DOWN:
                        if self.rooms:
                            self.selected_room_idx = min(self.selected_room_idx+1, len(self.rooms)-1)
                    elif ev.key == pygame.K_UP:
                        if self.rooms:
                            self.selected_room_idx = max(0, self.selected_room_idx-1)
                    elif ev.key == pygame.K_RETURN:
                        if self.rooms:
                            roomname = self.rooms[self.selected_room_idx]["name"]
                            self.send({"type":"join_room","room":roomname})
                            self.room = roomname
                    elif ev.key == pygame.K_BACKSPACE:
                        self.room_name_input = self.room_name_input[:-1]
                    else:
                        if ev.unicode:
                            self.room_name_input += ev.unicode

                elif self.state == STATE_GAME:
                    if ev.key == pygame.K_BACKSPACE:
                        self.chat_input = self.chat_input[:-1]
                    elif ev.key == pygame.K_RETURN:
                        if self.chat_input.strip() and self.room:
                            self.send({"type":"chat","room":self.room,"text":self.chat_input.strip()})
                            self.chat_input = ""
                    else:
                        if ev.unicode:
                            self.chat_input += ev.unicode

            if ev.type == pygame.MOUSEBUTTONDOWN:
                mx,my = ev.pos

                if self.state == STATE_LOGIN:
                    if 120<=mx<=320 and 96<=my<=122:
                        self.login_focus = "nick"
                    elif 120<=mx<=320 and 136<=my<=162:
                        self.login_focus = "password"
                    # buttons
                    if 40<=mx<=160 and 240<=my<=274:
                        self.attempt_login()
                    if 180<=mx<=300 and 240<=my<=274:
                        self.attempt_register()

                elif self.state == STATE_LOBBY:
                    if 460<=mx<=550 and 130<=my<=166:
                        if self.room_name_input.strip():
                            self.send({"type":"create_room","name":self.room_name_input.strip()})
                            self.room_name_input = ""
                    if 580<=mx<=670 and 130<=my<=166:
                        if self.rooms:
                            roomname = self.rooms[self.selected_room_idx]["name"]
                            self.send({"type":"join_room","room":roomname})
                            self.room = roomname
                    if 460<=mx<=670 and 180<=my<=216:
                        self.send({"type":"list_rooms"})
                    if 460<=mx<=670 and 230<=my<=266:
                        if self.net:
                            self.net.close()
                            self.net = None
                            self.state = STATE_LOGIN

                elif self.state == STATE_GAME:
                    # Board coords (must match draw_game)
                    board_x0 = 20 + MARGIN
                    board_y0 = 20 + MARGIN
                    board_px = CELL * BOARD_SIZE

                    # Click inside board?
                    if board_x0 <= mx < board_x0 + board_px and board_y0 <= my < board_y0 + board_px:
                        col = (mx - board_x0) // CELL
                        row = (my - board_y0) // CELL
                        # bounds check (should always be true here)
                        if 0 <= row < BOARD_SIZE and 0 <= col < BOARD_SIZE and self.room:
                            # send move (col,row correspond to x,y)
                            self.send({"type":"move","room":self.room,"x":int(col),"y":int(row)})

                    # Panel positions (must match draw_game)
                    panel_x = board_x0 + board_px + 40
                    # Chat send button
                    if panel_x+300 <= mx <= panel_x+380 and 578 <= my <= 610:
                        if self.chat_input.strip() and self.room:
                            self.send({"type":"chat","room":self.room,"text":self.chat_input.strip()})
                            self.chat_input = ""

                    # Leave
                    if panel_x+20 <= mx <= panel_x+140 and 620 <= my <= 652:
                        if self.room:
                            self.send({"type":"leave_room","room":self.room})
                            self.room = None
                            self.state = STATE_LOBBY

                    # Surrender (show confirm popup)
                    if panel_x+160 <= mx <= panel_x+280 and 620 <= my <= 652:
                        if self.room:
                            self.confirm_surrender = True
                    # if confirm popup visible handle its buttons (Yes / No)
                    if self.confirm_surrender:
                        # popup geometry (match draw_game)
                        pw, ph = 360, 140
                        px = (WINDOW_W - pw)//2
                        py = (WINDOW_H - ph)//2
                        # Yes
                        if px+40 <= mx <= px+140 and py+80 <= my <= py+120:
                            # send surrender
                            if self.room:
                                self.send({"type":"surrender","room":self.room})
                            self.confirm_surrender = False
                        # No
                        if px+220 <= mx <= px+320 and py+80 <= my <= py+120:
                            self.confirm_surrender = False

        if self.state == STATE_LOBBY:
            if self.room:
                self.state = STATE_GAME

    def render(self):
        if self.state == STATE_LOGIN:
            self.draw_login()
        elif self.state == STATE_LOBBY:
            self.draw_lobby()
        elif self.state == STATE_GAME:
            self.draw_game()
        pygame.display.flip()

def mainloop():
    app = App()
    clock = pygame.time.Clock()
    app.nick = f"Player{int(time.time())%1000}"
    while True:
        app.update()
        app.render()
        clock.tick(30)

if __name__=="__main__":
    mainloop()
