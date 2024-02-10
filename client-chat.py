# import socket
# import concurrent.futures

# def connect_to_server(number):
#     print(number)
#     server_address = ('127.0.0.1', 8989)
#     message = f"/home/liad12345/testfiles/test{number}.txt\n"
#     try:
#         # Create a TCP/IP socket
#         sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#         # Connect the socket to the server
#         sock.connect(server_address)
#         print(f"Connected to {server_address}")
#         # Send the message to the server
#         sock.sendall(message.encode())
#         received_data = b""
#         while True:
#             chunk = sock.recv(4096)  # Receive 4KB at a time
#             if not chunk:
#                 break
#     except Exception as e:
#         print(f"Failed to connect to {server_address}: {e}")
#     finally:
#         # Close the socket
#         sock.close()

# # Create a ThreadPoolExecutor with maximum 20 threads
# with concurrent.futures.ThreadPoolExecutor(max_workers=20) as executor:
#     for i in range(20):
#         executor.submit(connect_to_server, (i % 6) + 1)



# great for testing sudden client connection:

import socket
import threading

def connect_to_server(number):
    print(number)
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
        # received_data = b""
        # while True:
        #     chunk = sock.recv(4096)  # Receive 1KB at a time
        #     if not chunk:
        #         break
    except Exception as e:
        print(f"Failed to connect to {server_address}: {e}")
    finally:
        # Close the socket
        sock.close()

# Create 50 threads to establish connections
threads = []

for i in range(1):
    t = threading.Thread(target=connect_to_server(6))
    threads.append(t)
    t.start()

# Wait for all threads to complete
for t in threads:
    t.join()