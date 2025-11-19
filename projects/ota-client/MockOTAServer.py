import http.server
import socketserver
import json
import os
import hashlib
import ssl
from datetime import datetime

# --- Configuração do Servidor ---
HTTPS_PORT = 8443 # Porta padrão para HTTPS
SERVER_ADDRESS = "127.0.0.1" # Use 0.0.0.0 para acessar de outras máquinas

# Nome do arquivo de firmware que será servido
FIRMWARE_FILE = "firmware_v1.1.0.bin"

# Certificados gerados pelo OpenSSL
CERT_FILE = "cert.pem" 
KEY_FILE = "key.pem"

# Nome do arquivo de assinatura (SIG)
SIGNATURE_FILE = "firmware_v1.1.0.sig"

# Detalhes da nova versão
LATEST_VERSION = {
    "version": "1.1.0", 
    "url": f"https://{SERVER_ADDRESS}:{HTTPS_PORT}/{FIRMWARE_FILE}",
    "signature_url": f"https://{SERVER_ADDRESS}:{HTTPS_PORT}/{SIGNATURE_FILE}", 
    "hash": "6349a23f7160ae1b47ef5016c4f3929a736b5f19417d902d1dadfe5b96e668c5" 
}

class OTAServerHandler(http.server.SimpleHTTPRequestHandler):
    """Manipulador de requisições que simula os endpoints OTA."""

    def do_GET(self):
        """Processa requisições GET."""
        
        # 1. Endpoint de Verificação de Versão
        if self.path == "/api/firmware/latest":
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Cliente requisitou: {self.path}")

            response_json = json.dumps(LATEST_VERSION)
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Content-Length', str(len(response_json)))
            self.send_header('Connection', 'close')
            self.end_headers()
            
            response_json = json.dumps(LATEST_VERSION)
            self.wfile.write(response_json.encode('utf-8'))
            print(f"   -> Resposta JSON enviada: {response_json}")
            
        # 2. Endpoint de Download do Firmware
        elif self.path == f"/{FIRMWARE_FILE}":
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Cliente requisitou download: {self.path}")

            if not os.path.exists(FIRMWARE_FILE):
                self.send_error(404, "Arquivo de firmware não encontrado no servidor.")
                return

            self.send_response(200)
            self.send_header('Content-type', 'application/octet-stream')
            self.send_header('Content-Disposition', f'attachment; filename="{FIRMWARE_FILE}"')
            self.send_header('Content-Length', str(os.path.getsize(FIRMWARE_FILE)))
            self.send_header('Connection', 'close')
            self.end_headers()
            
            with open(FIRMWARE_FILE, 'rb') as f:
                self.wfile.write(f.read())
            print(f"   -> Arquivo {FIRMWARE_FILE} enviado com sucesso.")
            
        # 3. Endpoint de Download da Assinatura
        elif self.path == f"/{SIGNATURE_FILE}":
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Cliente requisitou assinatura: {self.path}")

            if not os.path.exists(SIGNATURE_FILE):
                self.send_error(404, "Arquivo de assinatura não encontrado.")
                return

            self.send_response(200)
            self.send_header('Content-type', 'application/octet-stream')
            self.send_header('Content-Disposition', f'attachment; filename="{SIGNATURE_FILE}"')
            self.send_header('Content-Length', str(os.path.getsize(SIGNATURE_FILE))) 
            self.send_header('Connection', 'close') 
            self.end_headers()

            with open(SIGNATURE_FILE, 'rb') as f:
                self.wfile.write(f.read())
            print(f"   -> Arquivo {SIGNATURE_FILE} enviado com sucesso.")
            
        else:
            # Para outros caminhos, usa o comportamento padrão do SimpleHTTPRequestHandler
            http.server.SimpleHTTPRequestHandler.do_GET(self)

print(f"--- Servidor Mock OTA ---")
print(f"Iniciando servidor em http://{SERVER_ADDRESS}:{HTTPS_PORT}")
print(f"Endpoint de Versão: http://{SERVER_ADDRESS}:{HTTPS_PORT}/api/firmware/latest")
print(f"Arquivo de Firmware: {FIRMWARE_FILE}")
print(f"Hash SHA256 Esperado: {LATEST_VERSION['hash']}")

try:
    # Use o HTTPServer, que lida melhor com o handshake SSL
    httpd = http.server.HTTPServer((SERVER_ADDRESS, HTTPS_PORT), OTAServerHandler)
    
    # 1. Configurar o contexto SSL
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=CERT_FILE, keyfile=KEY_FILE)
    
    # 2. Atribuir o contexto ao servidor
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
    
    print(f"Servidor HTTPS rodando em https://{SERVER_ADDRESS}:{HTTPS_PORT}")
    httpd.serve_forever()

except FileNotFoundError:
    print(f"\nERRO: Arquivos de certificado ({CERT_FILE} ou {KEY_FILE}) não encontrados.")
    print("Execute o comando OpenSSL para gerá-los antes de rodar o servidor.")
    exit(1)
except KeyboardInterrupt:
    print("\nServidor parado pelo usuário.")
    exit(0)