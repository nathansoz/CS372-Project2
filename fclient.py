#!/usr/bin/python3

import sys
import os
import socket
import argparse


def send_int(sock: socket, value: int) -> bool:
    message = value.to_bytes(4, byteorder='big')
    message_length = len(message)
    to_send = message_length.to_bytes(4, byteorder='big')

    len_sent = 0
    while len_sent < len(to_send):
        try:
            sent = sock.send(to_send[len_sent:])
        except BrokenPipeError:
            print("Connection has been closed.")
            return False
        if sent == 0:
            print("Connection has been closed.")
            return False
        len_sent += sent

    len_sent = 0
    while len_sent < message_length:
        try:
            sent = sock.send(message[len_sent:])
        except BrokenPipeError:
            print("Connection has been closed.")
            return False
        if sent == 0:
            print("Connection has been closed.")
            return False
        len_sent += sent


def send_message(sock: socket, message: str) -> bool:
    send_string = message.encode('utf-8')
    message_length = len(send_string)
    to_send = message_length.to_bytes(4, byteorder='big')

    len_sent = 0
    while len_sent < len(to_send):
        try:
            sent = sock.send(to_send[len_sent:])
        except BrokenPipeError:
            print("Connection has been closed.")
            return False
        if sent == 0:
            print("Connection has been closed.")
            return False
        len_sent += sent

    len_sent = 0
    while len_sent < message_length:
        try:
            sent = sock.send(send_string[len_sent:])
        except BrokenPipeError:
            print("Connection has been closed.")
            return False
        if sent == 0:
            print("Connection has been closed.")
            return False
        len_sent += sent


def recv_message(sock: socket) -> str:
    recieved = []
    bytes_recieved = 0

    while bytes_recieved < 4:
        got = sock.recv(4 - bytes_recieved)
        if got == b'':
            print("Connection has been closed.")
            return ""
        recieved.append(got)
        bytes_recieved += len(got)

    message_size = int.from_bytes(b''.join(recieved), byteorder='big')

    recieved.clear()
    bytes_recieved = 0
    while bytes_recieved < message_size:
        got = sock.recv(message_size - bytes_recieved)
        if got == b'':
            print("Connection has been closed.")
            return ""
        recieved.append(got)
        bytes_recieved += len(got)

    return b''.join(recieved).decode('utf-8')


def recv_file(sock: socket, filename: str):
    recieved = []
    bytes_recieved = 0

    while bytes_recieved < 4:
        got = sock.recv(4 - bytes_recieved)
        if got == b'':
            print("Connection has been closed.")
            return ""
        recieved.append(got)
        bytes_recieved += len(got)

    message_size = int.from_bytes(b''.join(recieved), byteorder='big')

    recieved.clear()
    bytes_recieved = 0
    while bytes_recieved < message_size:
        got = sock.recv(message_size - bytes_recieved)
        if got == b'':
            print("Connection has been closed.")
            return ""
        recieved.append(got)
        bytes_recieved += len(got)

    with open(filename, 'wb') as file:
        file.write(b''.join(recieved))
        file.flush()
        os.fsync(file.fileno())


def connect_server(server: str, port: str):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((server, port))
    return s


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Connects to a server and allows listing files.")
    parser.add_argument('--server', required=True)
    parser.add_argument('--port', required=True, type=int)
    parser.add_argument('--dataport', required=True, type=int)

    (args, command) = parser.parse_known_args()

    command = ' '.join(command)

    # Connect to the server and give it our data port for future use
    print("Connecting to {0} on port {1}".format(args.server, args.port))
    sock = connect_server(args.server, args.port)
    send_int(sock, args.dataport)

    # Set up our server in case we need it
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('', args.dataport))
    server.listen(1)

    send_message(sock, command)
    msg = recv_message(sock)

    if msg == "dir":
        (newsock, addr) = server.accept()
        listing = recv_message(newsock)
        while listing != "__DIR_LISTING_FINISHED__":
            print(listing)
            listing = recv_message(newsock)

        newsock.close()
    elif msg == "transfer":
        (newsock, addr) = server.accept()
        recv_file(newsock, command[3:])
        newsock.close()
    else:
        print(msg)

    sock.close()
    server.close()

