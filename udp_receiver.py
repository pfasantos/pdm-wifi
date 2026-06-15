import socket
import os

# Configurações (Devem bater com o main.h)
UDP_IP = "0.0.0.0"
UDP_PORT = 8888
BUFFER_SIZE = 65536
OUTPUT_DIR = "resultados"
FILE_PREFIX = "udp5min"  # <-- Edit this prefix

RECV_BUF_SIZE = 5 * 1024 * 1024

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

def run_receiver():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    output_file = get_output_filename(OUTPUT_DIR, FILE_PREFIX)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    try:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, RECV_BUF_SIZE)
        actual_buf = sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        print(f"[*] Buffer de recepção do OS ajustado para: {actual_buf/1024/1024:.2f} MB")
    except Exception as e:
        print(f"[!] Aviso: Não foi possível aumentar o buffer do OS: {e}")

    print(f"[*] Escutando em {UDP_IP}:{UDP_PORT}...")
    print(f"[*] Gravando em {output_file}...")

    total_bytes = 0
    packet_count = 0

    with open(output_file, "wb") as f:
        try:
            while True:
                sock.settimeout(15.0)
                data, addr = sock.recvfrom(BUFFER_SIZE)
                if not data:
                    break

                f.write(data)
                total_bytes += len(data)
                packet_count += 1

                if packet_count % 100 == 0:
                    print(f"[*] Recebidos: {packet_count} pacotes ({total_bytes / 1024:.2f} KB)")

        except socket.timeout:
            print("\n[!] Timeout: Nenhum dado recebido por 15s. Encerrando...")
        except KeyboardInterrupt:
            print("\n[*] Interrompido pelo usuário.")
        finally:
            sock.close()

    print(f"\n--- Resumo ---")
    print(f"Arquivo salvo: {output_file}")
    print(f"Total de bytes recebidos: {total_bytes}")
    print(f"Total de pacotes: {packet_count}")
    print(f"Tamanho esperado (743 pacotes): {743 * 4088} bytes")
    if total_bytes > 0:
        loss = (1 - (total_bytes / (743 * 4088))) * 100
        print(f"Perda estimada: {loss:.2f}%")

if __name__ == "__main__":
    run_receiver()
