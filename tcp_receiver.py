import socket
import os

# Configurações (Devem bater com o main.h)
TCP_IP = "0.0.0.0"
TCP_PORT = 8888
CHUNK_SIZE = 4088
OUTPUT_DIR = "resultados"
FILE_PREFIX = "tcp5min"  # <-- Edit this prefix

def get_output_filename(directory, prefix):
    """Returns the next available filename: prefix.raw, prefix-1.raw, prefix-2.raw..."""
    base = os.path.join(directory, f"{prefix}.raw")
    if not os.path.exists(base):
        return base
    counter = 1
    while True:
        candidate = os.path.join(directory, f"{prefix}-{counter}.raw")
        if not os.path.exists(candidate):
            return candidate
        counter += 1

def recv_exact(sock, n):
    """Read exactly n bytes from a stream socket."""
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            return None
        buf.extend(chunk)
    return bytes(buf)

def run_receiver():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    output_file = get_output_filename(OUTPUT_DIR, FILE_PREFIX)

    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind((TCP_IP, TCP_PORT))
    server_sock.listen(1)
    print(f"[*] Aguardando conexão em {TCP_IP}:{TCP_PORT}...")
    conn, addr = server_sock.accept()
    print(f"[*] ESP32 conectado: {addr}")
    print(f"[*] Gravando em {output_file}...")
    conn.settimeout(15.0)
    total_bytes = 0
    packet_count = 0
    with open(output_file, "wb") as f:
        try:
            while True:
                data = recv_exact(conn, CHUNK_SIZE)
                if data is None:
                    print("\n[!] Conexão encerrada pelo ESP32.")
                    break
                f.write(data)
                total_bytes += len(data)
                packet_count += 1
                if packet_count % 100 == 0:
                    print(f"[*] Recebidos: {packet_count} blocos ({total_bytes / 1024:.2f} KB)")
        except socket.timeout:
            print("\n[!] Timeout: Nenhum dado recebido por 15s. Encerrando...")
        except KeyboardInterrupt:
            print("\n[*] Interrompido pelo usuário.")
        finally:
            conn.close()
            server_sock.close()

    print(f"\n--- Resumo ---")
    print(f"Arquivo salvo: {output_file}")
    print(f"Total de bytes recebidos: {total_bytes}")
    print(f"Total de blocos: {packet_count}")
    expected = 743 * CHUNK_SIZE
    print(f"Tamanho esperado (743 blocos): {expected} bytes")
    if total_bytes > 0:
        loss = (1 - (total_bytes / expected)) * 100
        print(f"Perda estimada: {loss:.2f}%")

if __name__ == "__main__":
    run_receiver()
