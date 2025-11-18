import socket, threading, json, os, traceback, time

HOST = "0.0.0.0"
PORT = 9009
BOARD_SIZE = 15
RANK_FILE = "ranks.json"

clients_lock = threading.Lock()
clients = set()  # set of ClientHandler

rooms = {}  # name -> room dict

users_lock = threading.Lock()
if os.path.exists("users.json"):
    with open("users.json","r") as f:
        users = json.load(f)
else:
    users = {}

if os.path.exists(RANK_FILE):
    with open(RANK_FILE,"r") as f:
        ranks = json.load(f)
else:
    ranks = {}

def save_users():
    with open("users.json","w") as f:
        json.dump(users,f)

def save_ranks():
    with open(RANK_FILE,"w") as f:
        json.dump(ranks,f)

def send_line(conn, obj):
    try:
        data = json.dumps(obj, separators=(',',':')) + "\n"
        conn.sendall(data.encode())
    except Exception:
        pass

class ClientHandler(threading.Thread):
    def __init__(self, conn, addr):
        super().__init__(daemon=True)
        self.conn = conn
        self.addr = addr
        self.nick = None
        self.logged_in = False
        self.current_room = None
        self.running = True

    def run(self):
        buf = b""
        try:
            while self.running:
                data = self.conn.recv(4096)
                if not data:
                    break
                buf += data
                while b'\n' in buf:
                    line, buf = buf.split(b'\n',1)
                    try:
                        msg = json.loads(line.decode())
                        self.handle_msg(msg)
                    except Exception:
                        traceback.print_exc()
                        send_line(self.conn, {"type":"error","message":"invalid_json"})
        except Exception:
            traceback.print_exc()
        finally:
            self.cleanup()

    def handle_msg(self, msg):
        t = msg.get("type")
        if t == "register":
            nick = msg.get("nick"); pwd = msg.get("password")
            if not nick or not pwd:
                send_line(self.conn, {"type":"register fail","reason":"missing"})
                return
            with users_lock:
                if nick in users:
                    send_line(self.conn, {"type":"register_fail","reason":"exists"})
                else:
                    users[nick] = {"password":pwd}
                    save_users()
                    ranks.setdefault(nick, {"wins":0,"losses":0})
                    save_ranks()
                    send_line(self.conn, {"type":"register ok","nick":nick})
        elif t == "login":
            nick = msg.get("nick"); pwd = msg.get("password")
            with users_lock:
                if nick in users and users[nick]["password"] == pwd:
                    self.nick = nick
                    self.logged_in = True
                    ranks.setdefault(nick, {"wins":0,"losses":0})
                    save_ranks()
                    send_line(self.conn, {"type":"login_ok","nick":nick,"rank":ranks[nick]})
                    self.send_rooms_list()
                else:
                    send_line(self.conn, {"type":"login_fail","reason":"bad_credentials"})
        elif t == "identify":
            self.nick = msg.get("nick", f"Guest{self.addr[1]}")
            self.logged_in = True
            ranks.setdefault(self.nick, {"wins":0,"losses":0})
            save_ranks()
            send_line(self.conn, {"type":"identified","nick":self.nick,"rank":ranks[self.nick]})
            self.send_rooms_list()
        elif t == "list_rooms":
            self.send_rooms_list()
        elif t == "create_room":
            name = msg.get("name") or f"Room-{int(time.time())%10000}"
            self.create_room(name)
        elif t == "join_room":
            name = msg.get("room")
            self.join_room(name)
        elif t == "leave_room":
            self.leave_current_room()
        elif t == "move":
            room = msg.get("room")
            x = int(msg.get("x")); y = int(msg.get("y"))
            self.handle_move(room, x, y)
        elif t == "chat":
            room = msg.get("room"); text = msg.get("text","")
            self.handle_chat(room, text)
        elif t == "restart":
            room = msg.get("room")
            self.handle_restart(room)
        elif t == "surrender":
            room = msg.get("room")
            self.handle_surrender(room)
        else:
            send_line(self.conn, {"type":"error","message":"unknown_type"})

    # --- Room and game handling ---
    def send_rooms_list(self):
        with clients_lock:
            lst = [{"name":r["name"], "players":len(r["players"]), "status":r["status"]} for r in rooms.values()]
        send_line(self.conn, {"type":"rooms","rooms":lst})

    def create_room(self, name):
        if name in rooms:
            send_line(self.conn, {"type":"error","message":"room_exists"})
            return
        room = {
            "name": name,
            "players": [self],
            "nicks": [self.nick],
            "board": [[0 for _ in range(BOARD_SIZE)] for __ in range(BOARD_SIZE)],
            "turn": None,
            "status": "waiting",
            "winner": None
        }
        rooms[name] = room
        self.current_room = name
        send_line(self.conn, {"type":"room_joined","room":name,"players":room["nicks"]})
        self.broadcast_rooms()

    def join_room(self, name):
        room = rooms.get(name)
        if not room:
            send_line(self.conn, {"type":"error","message":"room not found"})
            return
        if len(room["players"]) >= 2:
            send_line(self.conn, {"type":"error","message":"room full"})
            return
        room["players"].append(self)
        room["nicks"].append(self.nick)
        room["status"] = "playing"
        room["turn"] = room["nicks"][0]
        self.current_room = name
        self.broadcast_room_state(name)
        self.broadcast_rooms()

    def leave_current_room(self):
        if not self.current_room:
            return
        self._remove_from_room(self.current_room)
        self.current_room = None
        self.send_rooms_list()
        self.broadcast_rooms()

    def _remove_from_room(self, name):
        room = rooms.get(name)
        if not room:
            return
        if self in room["players"]:
            idx = room["players"].index(self)
            room["players"].pop(idx)
            room["nicks"].pop(idx)
        if not room["players"]:
            rooms.pop(name, None)
        else:
            if len(room["players"]) == 1:
                room["status"] = "waiting"
                room["turn"] = room["nicks"][0]
                room["board"] = [[0 for _ in range(BOARD_SIZE)] for __ in range(BOARD_SIZE)]
                room["winner"] = None
            self.broadcast_room_state(name)

    def handle_move(self, room_name, x, y):
        room = rooms.get(room_name)
        if not room:
            send_line(self.conn, {"type":"error","message":"room not found"})
            return
        if room["status"] != "playing":
            send_line(self.conn, {"type":"error","message":"not playing"})
            return
        if self.nick != room["turn"]:
            send_line(self.conn, {"type":"error","message":"not your turn"})
            return
        if x < 0 or y < 0 or x >= BOARD_SIZE or y >= BOARD_SIZE:
            send_line(self.conn, {"type":"error","message":"invalid_move"})
            return
        if room["board"][y][x] != 0:
            send_line(self.conn, {"type":"error","message":"cell_occupied"})
            return

        mark = 1 if room["nicks"].index(self.nick) == 0 else 2
        room["board"][y][x] = mark

        winner = check_win(room["board"], x, y, mark)
        if winner:
            room["status"] = "won"
            room["winner"] = self.nick
            loser = None
            for n in room["nicks"]:
                if n != self.nick:
                    loser = n
            ranks.setdefault(self.nick, {"wins":0,"losses":0})
            ranks[self.nick]["wins"] += 1
            if loser:
                ranks.setdefault(loser, {"wins":0,"losses":0})
                ranks[loser]["losses"] += 1
            save_ranks()
        else:
            full = all(all(c!=0 for c in row) for row in room["board"])
            if full:
                room["status"] = "draw"
            else:
                room["turn"] = room["nicks"][1] if room["turn"] == room["nicks"][0] else room["nicks"][0]

        self.broadcast_room_state(room_name)

        if room["status"] in ("won","draw"):
            for p in room["players"]:
                send_line(p.conn, {"type":"info","message":"Match ended"})
                send_line(p.conn, {"type":"rank_update","rank":ranks.get(p.nick,{"wins":0,"losses":0})})
            time.sleep(1)
            self.handle_restart(room_name)

    def handle_chat(self, room_name, text):
        room = rooms.get(room_name)
        if not room:
            send_line(self.conn, {"type":"error","message":"room not found"})
            return
        payload = {"type":"chat","room":room_name,"from":self.nick,"text":text}
        for p in room["players"]:
            send_line(p.conn, payload)

    def handle_restart(self, room_name):
        room = rooms.get(room_name)
        if not room:
            return
        room["board"] = [[0 for _ in range(BOARD_SIZE)] for __ in range(BOARD_SIZE)]
        room["status"] = "playing"
        room["winner"] = None
        room["turn"] = room["nicks"][0]
        self.broadcast_room_state(room_name)

    def handle_surrender(self, room_name):
        room = rooms.get(room_name)
        if not room or room["status"] != "playing":
            send_line(self.conn, {"type":"error","message":"room not playing"})
            return
        opponent = None
        for p in room["players"]:
            if p != self:
                opponent = p
                break
        room["status"] = "won"
        room["winner"] = opponent.nick if opponent else None

        ranks.setdefault(self.nick, {"wins":0,"losses":0})
        if opponent:
            ranks.setdefault(opponent.nick, {"wins":0,"losses":0})
        ranks[self.nick]["losses"] += 1
        if opponent:
            ranks[opponent.nick]["wins"] += 1
        save_ranks()

        for p in room["players"]:
            if p == self:
                send_line(p.conn, {"type":"info","message":"you have surrendered"})
            else:
                send_line(p.conn, {"type":"info","message":f"Player {self.nick} has surrendered. You win!"})
            send_line(p.conn, {"type":"rank update","rank":ranks.get(p.nick,{"wins":0,"losses":0})})

        time.sleep(1)
        self.handle_restart(room_name)

    def broadcast_room_state(self, room_name):
        room = rooms.get(room_name)
        if not room:
            return
        payload = {"type":"state","room":room_name,"board":room["board"],
                   "turn":room["turn"],"players":room["nicks"],
                   "status":room["status"],"winner":room.get("winner")}
        for p in room["players"]:
            send_line(p.conn, payload)

    def broadcast_rooms(self):
        with clients_lock:
            lst = [{"name":r["name"], "players":len(r["players"]), "status":r["status"]} for r in rooms.values()]
            for c in list(clients):
                send_line(c.conn, {"type":"rooms","rooms":lst})

    def cleanup(self):
        try:
            with clients_lock:
                clients.discard(self)
        except Exception:
            pass
        if self.current_room:
            self._remove_from_room(self.current_room)
        try:
            self.conn.close()
        except Exception:
            pass

# --- Game logic ---
def check_win(board, x, y, mark):
    N = len(board); dirs = [(1,0),(0,1),(1,1),(1,-1)]
    for dx,dy in dirs:
        cnt = 1
        nx,ny = x+dx,y+dy
        while 0<=nx<N and 0<=ny<N and board[ny][nx]==mark:
            cnt+=1; nx+=dx; ny+=dy
        nx,ny = x-dx,y-dy
        while 0<=nx<N and 0<=ny<N and board[ny][nx]==mark:
            cnt+=1; nx-=dx; ny-=dy
        if cnt>=5:
            return True
    return False

# --- Server accept loop ---
def accept_loop(server_sock):
    print(f"Server listening on {HOST}:{PORT}")
    while True:
        conn, addr = server_sock.accept()
        print("Accepted", addr)
        handler = ClientHandler(conn, addr)
        with clients_lock:
            clients.add(handler)
        handler.start()

def main():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((HOST, PORT))
    srv.listen(32)
    try:
        accept_loop(srv)
    except KeyboardInterrupt:
        print("Shutting down server")
    finally:
        srv.close()

if __name__=="__main__":
    main()