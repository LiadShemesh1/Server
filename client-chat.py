import socket
import threading

def connect_to_server(number):
    server_address = ('127.0.0.1', 8989)
    message = f"/home/liad12345/testfiles/test{number}.txt\n"
    try:
        # Create a TCP/IP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Connect the socket to the server
        sock.connect(server_address)
        print(f"Connected to {server_address}")
        # Send the message to the server
        sock.sendall(message.encode())
    except Exception as e:
        print(f"Failed to connect to {server_address}: {e}")
    finally:
        # Close the socket
        sock.close()

# Create 50 threads to establish connections
threads = []
for i in range(20):
    t = threading.Thread(target=connect_to_server(i))
    threads.append(t)
    t.start()

# Wait for all threads to complete
for t in threads:
    t.join()