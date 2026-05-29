import socket
import os

# Configurações (Devem bater com o main.h)
UDP_IP = "0.0.0.0"  # Escuta em todas as interfaces
UDP_PORT = 8888
BUFFER_SIZE = 65536 # Buffer de leitura do socket (maior que o pacote)
OUTPUT_FILE = "resultados/teste_python"

# Tenta aumentar o buffer de recepção do Kernel do Linux
# Isso é CRÍTICO para evitar drops em pacotes fragmentados
RECV_BUF_SIZE = 4 * 1024 * 1024 # 4MB

def run_receiver():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    
    try:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, RECV_BUF_SIZE)
        actual_buf = sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        print(f"[*] Buffer de recepção do OS ajustado para: {actual_buf/1024/1024:.2f} MB")
    except Exception as e:
        print(f"[!] Aviso: Não foi possível aumentar o buffer do OS: {e}")

    print(f"[*] Escutando em {UDP_IP}:{UDP_PORT}...")
    print(f"[*] Gravando em {OUTPUT_FILE}...")

    if not os.path.exists("resultados"):
        os.makedirs("resultados")

    total_bytes = 0
    packet_count = 0
    
    with open(OUTPUT_FILE, "wb") as f:
        try:
            while True:
                # Timeout de 10 segundos se parar de receber
                sock.settimeout(10.0)
                data, addr = sock.recvfrom(BUFFER_SIZE)
                if not data:
                    break
                
                f.write(data)
                total_bytes += len(data)
                packet_count += 1
                
                if packet_count % 100 == 0:
                    print(f"[*] Recebidos: {packet_count} pacotes ({total_bytes / 1024:.2f} KB)")
                    
        except socket.timeout:
            print("\n[!] Timeout: Nenhum dado recebido por 10s. Encerrando...")
        except KeyboardInterrupt:
            print("\n[*] Interrompido pelo usuário.")
        finally:
            sock.close()

    print(f"\n--- Resumo ---")
    print(f"Total de bytes recebidos: {total_bytes}")
    print(f"Total de pacotes: {packet_count}")
    print(f"Tamanho esperado (743 pacotes): {743 * 4088} bytes")
    if total_bytes > 0:
        loss = (1 - (total_bytes / (743 * 4088))) * 100
        print(f"Perda estimada: {loss:.2f}%")

if __name__ == "__main__":
    run_receiver()
