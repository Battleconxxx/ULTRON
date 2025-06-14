import socket
import subprocess

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect("/tmp/ai.sock")

while True:
    data = b""
    while not data.endswith(b"\n"):
        chunk = sock.recv(1024)
        if not chunk:
            break
        data += chunk

    prompt = data.decode().strip()
    print(f"[GUEST] {prompt}")

    result = subprocess.run(
        ["ollama", "run", "tinyllama", "--prompt", prompt],
        capture_output=True, text=True
    )
    reply = result.stdout.strip()
    sock.sendall(reply.encode()[:1024])  # Send back a snippet
